module;

#include <any>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <termcolor/termcolor.hpp>

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
struct LetExpr;
struct Assignment;
struct Logical;
struct Call;
struct FnExpression;
struct Get;
struct Set;
struct SelfExpr;
struct Super;
// TODO: Move those that can be moved above Expression to avoid predeclaration.

using Expression = std::variant<Binary, Grouping, Literal, Unary, Ternary,
                                LetExpr, Assignment, Logical, Call, FnExpression, Get, Set, SelfExpr, Super>;

struct Super {
  Token keyword;
  Token method;
};

struct Get {
  std::unique_ptr<Expression> object;
  Token name;
};

struct Set {
  std::unique_ptr<Expression> object;
  Token name;
  std::unique_ptr<Expression> value;
};

struct SelfExpr {
  Token keyword;
};

struct BlockStatement;

struct FnExpression {
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

struct LetExpr {
  Token name;
};

struct Assignment {
  Token name;
  std::unique_ptr<Expression> value;
};

template <typename T> std::unique_ptr<Expression> make_expression(T &&expr) {
  return std::make_unique<Expression>(std::forward<T>(expr));
}

struct BlockStatement;
struct IfStatement;
struct WhileStatement;
struct ForStatement;

struct ReturnStatement {
  Token keyword;
  std::unique_ptr<Expression> value;
};

using Statement = std::variant<Expression, BlockStatement,
                               IfStatement, WhileStatement, ForStatement,
                               ReturnStatement>;

struct LetDeclaration {
  Token name;
  std::unique_ptr<Expression> initializer;
};

struct FnDeclaration {
  Token name;
  std::vector<Token> parameters;
  std::unique_ptr<BlockStatement> body;
  bool is_static = false;
};

struct StructDeclaration {
  Token name;
  std::unique_ptr<LetExpr> parent;
  std::vector<std::unique_ptr<FnDeclaration>> methods;
};

using Declaration = std::variant<LetDeclaration, Statement, FnDeclaration, StructDeclaration>;

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
  void print(std::ostream &os, const Program &program) const {
    os << termcolor::colorize;
    os << termcolor::bold << termcolor::bright_cyan << "Program"
       << termcolor::reset << "\n";

    for (size_t i = 0; i < program.declarations.size(); ++i) {
      bool is_last = (i == program.declarations.size() - 1);
      print_declaration(os, program.declarations[i], "", is_last);
    }
    os << termcolor::reset << "\n";
  }

  std::string print(const Program &program) const {
    std::ostringstream oss;
    print(oss, program);
    return oss.str();
  }

private:
  // Tree drawing characters
  static constexpr const char* BRANCH = "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ";  // "├── "
  static constexpr const char* LAST   = "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 ";  // "└── "
  static constexpr const char* PIPE   = "\xe2\x94\x82   ";                        // "│   "
  static constexpr const char* SPACE  = "    ";                                   // "    "

  void print_indent(std::ostream &os, const std::string &prefix, bool is_last) const {
    os << termcolor::bright_grey << prefix
       << (is_last ? LAST : BRANCH) << termcolor::reset;
  }

  std::string child_prefix(const std::string &prefix, bool is_last) const {
    return prefix + (is_last ? SPACE : PIPE);
  }

  // Node type label (bold, colored)
  void print_node(std::ostream &os, const std::string &type) const {
    os << termcolor::bold << termcolor::yellow << type << termcolor::reset;
  }

  // Keyword styling
  void print_keyword(std::ostream &os, std::string_view kw) const {
    os << termcolor::bold << termcolor::magenta << kw << termcolor::reset;
  }

  // Operator styling
  void print_operator(std::ostream &os, std::string_view op) const {
    os << termcolor::bold << termcolor::cyan << op << termcolor::reset;
  }

  // Identifier styling
  void print_identifier(std::ostream &os, std::string_view id) const {
    os << termcolor::bright_white << id << termcolor::reset;
  }

  // Literal value styling
  void print_literal_value(std::ostream &os, const std::any &value) const {
    if (!value.has_value()) {
      os << termcolor::bright_grey << "nil" << termcolor::reset;
      return;
    }
    if (value.type() == typeid(double)) {
      os << termcolor::bright_green << std::any_cast<double>(value) << termcolor::reset;
    } else if (value.type() == typeid(bool)) {
      os << termcolor::bright_blue
         << (std::any_cast<bool>(value) ? "true" : "false")
         << termcolor::reset;
    } else if (value.type() == typeid(std::string_view)) {
      os << termcolor::green << "\"" << std::any_cast<std::string_view>(value) << "\"" << termcolor::reset;
    } else if (value.type() == typeid(std::string)) {
      os << termcolor::green << "\"" << std::any_cast<std::string>(value) << "\"" << termcolor::reset;
    } else {
      os << termcolor::red << "<unknown>" << termcolor::reset;
    }
  }

  void print_expression(std::ostream &os, const Expression &expr,
                        const std::string &prefix, bool is_last) const {
    std::visit(
        overload{
            [&](const Binary &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Binary");
              os << " ";
              print_operator(os, e.op.lexeme());
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              print_expression(os, *e.left, new_prefix, false);
              print_expression(os, *e.right, new_prefix, true);
            },
            [&](const Grouping &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Group");
              os << "\n";
              print_expression(os, *e.expr, child_prefix(prefix, is_last), true);
            },
            [&](const Literal &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Literal");
              os << " ";
              print_literal_value(os, e.value);
              os << "\n";
            },
            [&](const Unary &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Unary");
              os << " ";
              print_operator(os, e.op.lexeme());
              os << "\n";
              print_expression(os, *e.right, child_prefix(prefix, is_last), true);
            },
            [&](const Ternary &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Ternary");
              os << " ";
              print_operator(os, "?:");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              print_indent(os, new_prefix, false);
              os << termcolor::bright_grey << "condition:" << termcolor::reset << "\n";
              print_expression(os, *e.condition, child_prefix(new_prefix, false), true);

              print_indent(os, new_prefix, false);
              os << termcolor::bright_grey << "then:" << termcolor::reset << "\n";
              print_expression(os, *e.true_expr, child_prefix(new_prefix, false), true);

              print_indent(os, new_prefix, true);
              os << termcolor::bright_grey << "else:" << termcolor::reset << "\n";
              print_expression(os, *e.false_expr, child_prefix(new_prefix, true), true);
            },
            [&](const LetExpr &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Var");
              os << " ";
              print_identifier(os, e.name.lexeme());
              os << "\n";
            },
            [&](const Assignment &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Assign");
              os << " ";
              print_identifier(os, e.name.lexeme());
              os << " ";
              print_operator(os, "=");
              os << "\n";
              print_expression(os, *e.value, child_prefix(prefix, is_last), true);
            },
            [&](const Logical &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Logical");
              os << " ";
              print_operator(os, e.op.lexeme());
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              print_expression(os, *e.left, new_prefix, false);
              print_expression(os, *e.right, new_prefix, true);
            },
            [&](const Call &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Call");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              bool has_args = !e.arguments.empty();

              print_indent(os, new_prefix, !has_args);
              os << termcolor::bright_grey << "callee:" << termcolor::reset << "\n";
              print_expression(os, *e.callee, child_prefix(new_prefix, !has_args), true);

              if (has_args) {
                print_indent(os, new_prefix, true);
                os << termcolor::bright_grey << "args:" << termcolor::reset << "\n";
                std::string args_prefix = child_prefix(new_prefix, true);
                for (size_t i = 0; i < e.arguments.size(); ++i) {
                  print_expression(os, *e.arguments[i], args_prefix,
                                   i == e.arguments.size() - 1);
                }
              }
            },
            [&](const FnExpression &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Lambda");
              os << " ";
              print_operator(os, "(");
              for (size_t i = 0; i < e.parameters.size(); ++i) {
                if (i > 0) os << termcolor::bright_grey << ", " << termcolor::reset;
                print_identifier(os, e.parameters[i].lexeme());
              }
              print_operator(os, ")");
              os << "\n";
              print_block(os, *e.body, child_prefix(prefix, is_last), true);
            },
            [&](const Get &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Get");
              os << " ";
              print_operator(os, ".");
              print_identifier(os, e.name.lexeme());
              os << "\n";
              print_expression(os, *e.object, child_prefix(prefix, is_last), true);
            },
            [&](const Set &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "Set");
              os << " ";
              print_operator(os, ".");
              print_identifier(os, e.name.lexeme());
              os << " ";
              print_operator(os, "=");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              print_indent(os, new_prefix, false);
              os << termcolor::bright_grey << "object:" << termcolor::reset << "\n";
              print_expression(os, *e.object, child_prefix(new_prefix, false), true);

              print_indent(os, new_prefix, true);
              os << termcolor::bright_grey << "value:" << termcolor::reset << "\n";
              print_expression(os, *e.value, child_prefix(new_prefix, true), true);
            },
            [&](const SelfExpr &) {
              print_indent(os, prefix, is_last);
              print_node(os, "Self");
              os << " ";
              print_keyword(os, "self");
              os << "\n";
            },
            [&](const Super &s) {
              print_indent(os, prefix, is_last);
              print_node(os, "Super");
              os << " ";
              print_keyword(os, "parent");
              print_operator(os, ".");
              print_identifier(os, s.method.lexeme());
              os << "\n";
            },
        },
        expr);
  }

  void print_block(std::ostream &os, const BlockStatement &b,
                   const std::string &prefix, bool is_last) const {
    print_indent(os, prefix, is_last);
    print_node(os, "Block");
    os << "\n";

    std::string new_prefix = child_prefix(prefix, is_last);
    for (size_t i = 0; i < b.declarations.size(); ++i) {
      print_declaration(os, b.declarations[i], new_prefix,
                        i == b.declarations.size() - 1);
    }
  }

  void print_statement(std::ostream &os, const Statement &statement,
                       const std::string &prefix, bool is_last) const {
    std::visit(
        overload{
            [&](const Expression &e) {
              print_indent(os, prefix, is_last);
              print_node(os, "ExprStmt");
              os << "\n";
              print_expression(os, e, child_prefix(prefix, is_last), true);
            },
            [&](const BlockStatement &b) {
              print_block(os, b, prefix, is_last);
            },
            [&](const IfStatement &i) {
              print_indent(os, prefix, is_last);
              print_node(os, "If");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);
              bool has_else = i.else_branch != nullptr;

              print_indent(os, new_prefix, false);
              os << termcolor::bright_grey << "condition:" << termcolor::reset << "\n";
              print_expression(os, *i.condition, child_prefix(new_prefix, false), true);

              print_indent(os, new_prefix, !has_else);
              os << termcolor::bright_grey << "then:" << termcolor::reset << "\n";
              print_statement(os, *i.then_branch, child_prefix(new_prefix, !has_else), true);

              if (has_else) {
                print_indent(os, new_prefix, true);
                os << termcolor::bright_grey << "else:" << termcolor::reset << "\n";
                print_statement(os, *i.else_branch, child_prefix(new_prefix, true), true);
              }
            },
            [&](const WhileStatement &w) {
              print_indent(os, prefix, is_last);
              print_node(os, "While");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);

              print_indent(os, new_prefix, false);
              os << termcolor::bright_grey << "condition:" << termcolor::reset << "\n";
              print_expression(os, *w.condition, child_prefix(new_prefix, false), true);

              print_indent(os, new_prefix, true);
              os << termcolor::bright_grey << "body:" << termcolor::reset << "\n";
              print_statement(os, *w.body, child_prefix(new_prefix, true), true);
            },
            [&](const ForStatement &f) {
              print_indent(os, prefix, is_last);
              print_node(os, "For");
              os << "\n";

              std::string new_prefix = child_prefix(prefix, is_last);

              if (f.initializer) {
                print_indent(os, new_prefix, false);
                os << termcolor::bright_grey << "init:" << termcolor::reset << "\n";
                print_declaration(os, *f.initializer, child_prefix(new_prefix, false), true);
              }

              if (f.condition) {
                print_indent(os, new_prefix, false);
                os << termcolor::bright_grey << "condition:" << termcolor::reset << "\n";
                print_expression(os, *f.condition, child_prefix(new_prefix, false), true);
              }

              if (f.increment) {
                print_indent(os, new_prefix, false);
                os << termcolor::bright_grey << "increment:" << termcolor::reset << "\n";
                print_expression(os, *f.increment, child_prefix(new_prefix, false), true);
              }

              print_indent(os, new_prefix, true);
              os << termcolor::bright_grey << "body:" << termcolor::reset << "\n";
              print_statement(os, *f.body, child_prefix(new_prefix, true), true);
            },
            [&](const ReturnStatement &r) {
              print_indent(os, prefix, is_last);
              print_node(os, "Return");
              os << "\n";
              if (r.value) {
                print_expression(os, *r.value, child_prefix(prefix, is_last), true);
              } else {
                print_indent(os, child_prefix(prefix, is_last), true);
                os << termcolor::bright_grey << "nil" << termcolor::reset << "\n";
              }
            },
        },
        statement);
  }

  void print_fn_declaration(std::ostream &os, const FnDeclaration &f,
                            const std::string &prefix, bool is_last) const {
    print_indent(os, prefix, is_last);
    print_node(os, "FnDecl");
    os << " ";
    if (f.is_static) {
      print_keyword(os, "static");
      os << " ";
    }
    print_identifier(os, f.name.lexeme());
    print_operator(os, "(");
    for (size_t i = 0; i < f.parameters.size(); ++i) {
      if (i > 0) os << termcolor::bright_grey << ", " << termcolor::reset;
      print_identifier(os, f.parameters[i].lexeme());
    }
    print_operator(os, ")");
    os << "\n";
    print_block(os, *f.body, child_prefix(prefix, is_last), true);
  }

  void print_declaration(std::ostream &os, const Declaration &declaration,
                         const std::string &prefix, bool is_last) const {
    std::visit(overload{
                   [&](const LetDeclaration &d) {
                     print_indent(os, prefix, is_last);
                     print_node(os, "LetDecl");
                     os << " ";
                     print_identifier(os, d.name.lexeme());
                     os << " ";
                     print_operator(os, "=");
                     os << "\n";
                     print_expression(os, *d.initializer, child_prefix(prefix, is_last), true);
                   },
                   [&](const Statement &s) {
                     print_statement(os, s, prefix, is_last);
                   },
                   [&](const FnDeclaration &f) {
                     print_fn_declaration(os, f, prefix, is_last);
                   },
                   [&](const StructDeclaration &c) {
                     print_indent(os, prefix, is_last);
                     print_node(os, "StructDecl");
                     os << " ";
                     print_identifier(os, c.name.lexeme());
                     if (c.parent) {
                       os << " ";
                       print_operator(os, ":");
                       os << " ";
                       print_identifier(os, c.parent->name.lexeme());
                     }
                     os << "\n";

                     std::string new_prefix = child_prefix(prefix, is_last);
                     for (size_t i = 0; i < c.methods.size(); ++i) {
                       print_fn_declaration(os, *c.methods[i], new_prefix,
                                            i == c.methods.size() - 1);
                     }
                   },
               },
               declaration);
  }
};

} // namespace seam::ast
