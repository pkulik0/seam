module;

#include <any>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

export module seam.ast;

import seam.common;
import seam.token;

using namespace seam::token;

export namespace seam::ast {

struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Ternary;
struct Variable;
struct Assignment;
struct Logical;
struct Call;
struct FunctionExpression;
struct Get;
struct Set;
struct ThisExpr;

using Expression = std::variant<Binary, Grouping, Literal, Unary, Ternary,
                                Variable, Assignment, Logical, Call, FunctionExpression, Get, Set, ThisExpr>;

struct Get {
  std::unique_ptr<Expression> object;
  Token name;
};

struct Set {
  std::unique_ptr<Expression> object;
  Token name;
  std::unique_ptr<Expression> value;
};

struct ThisExpr {
  Token keyword;
};

struct BlockStatement;

struct FunctionExpression {
  std::vector<Token> parameters;
  std::unique_ptr<BlockStatement> body;
};

struct Call {
  std::unique_ptr<Expression> callee;
  Token paren;
  std::vector<std::unique_ptr<Expression>> arguments;
};

struct Logical {
  std::unique_ptr<Expression> left;
  Token op;
  std::unique_ptr<Expression> right;
};

struct Binary {
  std::unique_ptr<Expression> left;
  Token op;
  std::unique_ptr<Expression> right;
};

struct Grouping {
  std::unique_ptr<Expression> expr;
};

struct Literal {
  std::any value;
};

struct Unary {
  Token op;
  std::unique_ptr<Expression> right;
};

struct Ternary {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Expression> true_expr;
  std::unique_ptr<Expression> false_expr;
};

struct Variable {
  Token name;
};

struct Assignment {
  Token name;
  std::unique_ptr<Expression> value;
};

template <typename T> std::unique_ptr<Expression> make_expression(T &&expr) {
  return std::make_unique<Expression>(std::forward<T>(expr));
}

struct PrintStatement {
  std::unique_ptr<Expression> expression;
};

struct BlockStatement;
struct IfStatement;
struct WhileStatement;
struct ForStatement;

struct ReturnStatement {
  Token keyword;
  std::unique_ptr<Expression> value;
};

using Statement = std::variant<PrintStatement, Expression, BlockStatement,
                               IfStatement, WhileStatement, ForStatement,
                               ReturnStatement>;

struct VariableDeclaration {
  Token name;
  std::unique_ptr<Expression> initializer;
};

struct FunctionDeclaration {
  Token name;
  std::vector<Token> parameters;
  std::unique_ptr<BlockStatement> body;
  bool is_static = false;
};

struct ClassDeclaration {
  Token name;
  std::vector<std::unique_ptr<FunctionDeclaration>> methods;
};

using Declaration = std::variant<VariableDeclaration, Statement, FunctionDeclaration, ClassDeclaration>;

struct IfStatement {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> then_branch;
  std::unique_ptr<Statement> else_branch;
};

struct WhileStatement {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> body;
};

struct ForStatement {
  std::unique_ptr<Declaration> initializer; 
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Expression> increment;
  std::unique_ptr<Statement> body;
};

struct BlockStatement {
  std::vector<Declaration> declarations;
};

struct Program {
  std::vector<Declaration> declarations;
};

class AstPrinter {
public:
  std::string print(const Program &program) const {
    std::string result;
    for (const auto &declaration : program.declarations) {
      result += print_declaration(declaration) + "\n";
    }
    return result;
  }

private:
  template <typename... Exprs>
  std::string parenthesize(std::string_view name, const Exprs &...exprs) const {
    std::string result = std::format("({}", name);
    ((result += std::format(" {}", print_expression(exprs))), ...);
    return result + ")";
  }

  std::string print_expression(const Expression &expr) const {
    return std::visit(
        overload{
            [this](const Binary &e) -> std::string {
              return parenthesize(e.op.lexeme(), *e.left, *e.right);
            },
            [this](const Grouping &e) -> std::string {
              return parenthesize("group", *e.expr);
            },
            [](const Literal &e) -> std::string {
              if (!e.value.has_value()) {
                return "nil";
              }
              if (e.value.type() == typeid(double)) {
                return std::format("{}", std::any_cast<double>(e.value));
              }
              if (e.value.type() == typeid(bool)) {
                return std::any_cast<bool>(e.value) ? "true" : "false";
              }
              if (e.value.type() == typeid(std::string_view)) {
                return std::string(std::any_cast<std::string_view>(e.value));
              }
              if (e.value.type() == typeid(std::string)) {
                return std::any_cast<std::string>(e.value);
              }
              return "unknown";
            },
            [this](const Unary &e) -> std::string {
              return parenthesize(e.op.lexeme(), *e.right);
            },
            [this](const Ternary &e) -> std::string {
              return parenthesize("?:", *e.condition, *e.true_expr,
                                  *e.false_expr);
            },
            [](const Variable &e) -> std::string {
              return std::string(e.name.lexeme());
            },
            [this](const Assignment &e) -> std::string {
              return parenthesize(e.name.lexeme(), *e.value);
            },
            [this](const Logical &e) -> std::string {
              return parenthesize(e.op.lexeme(), *e.left, *e.right);
            },
            [this](const Call &e) -> std::string {
              std::string result = std::format("({}", print_expression(*e.callee));
              for (const auto &arg : e.arguments) {
                result += std::format(" {}", print_expression(*arg));
              }
              return result + ")";
            },
            [this](const FunctionExpression &e) -> std::string {
              std::string params;
              for (const auto &param : e.parameters) {
                params += std::string(param.lexeme()) + ", ";
              }
              return std::format("fun({}) {}", params, print_block(*e.body));
            },
            [this](const Get &e) -> std::string {
              return std::format("({}.{})", print_expression(*e.object), e.name.lexeme());
            },
            [this](const Set &e) -> std::string {
              return std::format("({}.{}) = {}", print_expression(*e.object), e.name.lexeme(), print_expression(*e.value));
            },
            [](const ThisExpr &) -> std::string {
              return "this";
            },
        },
        expr);
  }
  std::string print_block(const BlockStatement &b) const {
    std::string result = "{\n";
    for (const auto &decl : b.declarations) {
      result += print_declaration(decl) + "\n";
    }
    return result + "}";
  }

  std::string print_statement(const Statement &statement) const {
    return std::visit(
        overload{
            [this](const PrintStatement &s) -> std::string {
              return std::format("print {}", print_expression(*s.expression));
            },
            [this](const Expression &e) -> std::string {
              return print_expression(e);
            },
            [this](const BlockStatement &b) -> std::string {
              return print_block(b);
            },
            [this](const IfStatement &i) -> std::string {
              std::string result =
                  std::format("if ({}) {}", print_expression(*i.condition),
                              print_statement(*i.then_branch));
              if (i.else_branch) {
                result +=
                    std::format(" else {}", print_statement(*i.else_branch));
              }
              return result;
            },
            [this](const WhileStatement &w) -> std::string {
              return std::format("while ({}) {}",
                                 print_expression(*w.condition),
                                 print_statement(*w.body));
            },
            [this](const ForStatement &f) -> std::string {
              std::string init_str = f.initializer ? print_declaration(*f.initializer) : "";
              std::string condition_str = f.condition ? print_expression(*f.condition) : "";
              std::string increment_str = f.increment ? print_expression(*f.increment) : "";
              return std::format("for ({}; {}; {}) {}", init_str, condition_str, increment_str, print_statement(*f.body));
            },
            [this](const ReturnStatement &r) -> std::string {
               std::string value = r.value ? print_expression(*r.value) : "nil";
               return std::format("return {}", value);
            },
        },
        statement);
  }

  std::string print_function_declaration(const FunctionDeclaration &f) const {
    std::string params;
    for (const auto &param : f.parameters) {
      params += std::string(param.lexeme()) + ", ";
    }
    return std::format("{}func {}({}) {}", f.is_static ? "static " : "", f.name.lexeme(), params, print_block(*f.body));
  }

  std::string print_declaration(const Declaration &declaration) const {
    return std::visit(overload{
                          [this](const VariableDeclaration &d) -> std::string {
                            return std::format(
                                "var {}: {}", d.name.lexeme(),
                                print_expression(*d.initializer));
                          },
                          [this](const Statement &s) -> std::string {
                            return print_statement(s);
                          },
                          [this](const FunctionDeclaration &f) -> std::string {
                            return print_function_declaration(f);
                          },
                          [this](const ClassDeclaration &c) -> std::string {
                            std::string methods;
                            for (const auto &method : c.methods) {
                              methods += std::format("{}", print_function_declaration(*method));
                            }
                            return std::format("class {}\n{{{}\n}}", c.name.lexeme(), methods);
                          },
                      },
                      declaration);
  }
};

} // namespace seam::ast
