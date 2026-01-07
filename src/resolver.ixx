module;

#include <format>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module seam.resolver;

import seam.ast;
import seam.common;
import seam.token;
import seam.interpreter;
import seam.error;

using namespace seam::ast;
using namespace seam::token;

export namespace seam::resolver {

class ResolutionError : public Error {
public:
  ResolutionError(Token token, std::string message)
      : m_token(std::move(token)), m_message(std::move(message)) {}

  [[nodiscard]] auto name() const noexcept -> std::string_view override {
    return "Resolution Error";
  }
  [[nodiscard]] auto reason() const noexcept -> std::string_view override {
    return m_message;
  }
  [[nodiscard]] auto location() const noexcept -> Location override {
    return {static_cast<int>(m_token.line()),
            static_cast<int>(m_token.column())};
  }

private:
  Token m_token;
  std::string m_message;
};

class Resolver {
private:
  enum class FunctionType {
    NONE,
    FUNCTION,
    METHOD,
    INITIALIZER,
    STATIC_METHOD
  };
  enum class ClassType { NONE, CLASS, SUBCLASS };

  interpreter::Interpreter &m_interpreter;
  std::vector<std::unordered_map<std::string, bool>> m_scopes;
  FunctionType m_current_function = FunctionType::NONE;
  ClassType m_current_class = ClassType::NONE;

  void begin_scope() { m_scopes.emplace_back(); }

  void end_scope() { m_scopes.pop_back(); }

  void declare(const Token &name) {
    if (m_scopes.empty())
      return;

    auto &scope = m_scopes.back();
    if (scope.contains(std::string{name.lexeme()})) {
      throw ResolutionError(name,
                            "Already a variable with this name in this scope.");
    }
    scope[std::string{name.lexeme()}] = false;
  }

  void define(const Token &name) {
    if (m_scopes.empty())
      return;
    m_scopes.back()[std::string{name.lexeme()}] = true;
  }

  void resolve_local(const Expression *expr, const Token &name) {
    for (int i = m_scopes.size() - 1; i >= 0; i--) {
      if (m_scopes[i].contains(std::string{name.lexeme()})) {
        m_interpreter.resolve(expr, m_scopes.size() - 1 - i);
        return;
      }
    }
  }

  void resolve(const Expression &expr) {
    const Expression *expr_ptr = &expr;
    std::visit(
        overload{
            [this](const Binary &e) -> void {
              resolve(*e.left);
              resolve(*e.right);
            },
            [this](const Grouping &e) -> void { resolve(*e.expr); },
            [](const Literal &) -> void {
              // Nothing to resolve
            },
            [this](const Unary &e) -> void { resolve(*e.right); },
            [this](const Ternary &e) -> void {
              resolve(*e.condition);
              resolve(*e.true_expr);
              resolve(*e.false_expr);
            },
            [this, expr_ptr](const Variable &e) -> void {
              if (!m_scopes.empty()) {
                const auto &scope = m_scopes.back();
                const auto it = scope.find(std::string{e.name.lexeme()});
                if (it != scope.end() && it->second == false) {
                  throw ResolutionError(
                      e.name,
                      "Can't read local variable in its own initializer.");
                }
              }
              resolve_local(expr_ptr, e.name);
            },
            [this, expr_ptr](const Assignment &e) -> void {
              resolve(*e.value);
              resolve_local(expr_ptr, e.name);
            },
            [this](const Logical &e) -> void {
              resolve(*e.left);
              resolve(*e.right);
            },
            [this](const Call &e) -> void {
              resolve(*e.callee);
              for (const auto &arg : e.arguments) {
                resolve(*arg);
              }
            },
            [this](const FunctionExpression &e) -> void {
              FunctionType enclosing_function = m_current_function;
              m_current_function = FunctionType::FUNCTION;

              begin_scope();
              for (const auto &param : e.parameters) {
                declare(param);
                define(param);
              }
              resolve_block(*e.body);
              end_scope();

              m_current_function = enclosing_function;
            },
            [this](const Get &e) -> void { resolve(*e.object); },
            [this](const Set &e) -> void {
              resolve(*e.value);
              resolve(*e.object);
            },
            [this, expr_ptr](const ThisExpr &e) -> void {
              if (m_current_class == ClassType::NONE) {
                throw ResolutionError(e.keyword,
                                      "Can't use 'this' outside of a class.");
              }
              if (m_current_function == FunctionType::STATIC_METHOD) {
                throw ResolutionError(e.keyword,
                                      "Can't use 'this' in a static method.");
              }
              resolve_local(expr_ptr, e.keyword);
            },
            [this, expr_ptr](const Super &e) -> void {
              if (m_current_class == ClassType::NONE) {
                throw ResolutionError(e.keyword,
                                      "Can't use 'super' outside of a class.");
              }
              if (m_current_class != ClassType::SUBCLASS) {
                throw ResolutionError(
                    e.keyword, "Can't use 'super' in a class with no superclass.");
              }
              resolve_local(expr_ptr, e.keyword);
            },
        },
        expr);
  }

  void resolve(const FunctionDeclaration &d, FunctionType type) {
    declare(d.name);
    define(d.name);

    FunctionType enclosing_function = m_current_function;
    m_current_function = type;

    begin_scope();
    for (const auto &param : d.parameters) {
      declare(param);
      define(param);
    }
    resolve_block(*d.body);
    end_scope();

    m_current_function = enclosing_function;
  }

  void resolve(const Statement &stmt) {
    std::visit(
        overload{
            [this](const PrintStatement &s) -> void { resolve(*s.expression); },
            [this](const Expression &e) -> void { resolve(e); },
            [this](const BlockStatement &b) -> void {
              begin_scope();
              resolve_block(b);
              end_scope();
            },
            [this](const IfStatement &i) -> void {
              resolve(*i.condition);
              resolve(*i.then_branch);
              if (i.else_branch) {
                resolve(*i.else_branch);
              }
            },
            [this](const WhileStatement &w) -> void {
              resolve(*w.condition);
              resolve(*w.body);
            },
            [this](const ForStatement &f) -> void {
              begin_scope();
              if (f.initializer) {
                resolve(*f.initializer);
              }
              resolve(*f.condition);
              if (f.increment) {
                resolve(*f.increment);
              }
              resolve(*f.body);
              end_scope();
            },
            [this](const ReturnStatement &r) -> void {
              if (m_current_function == FunctionType::NONE) {
                throw ResolutionError(r.keyword,
                                      "Can't return from top-level code.");
              }
              if (r.value) {
                if (m_current_function == FunctionType::INITIALIZER) {
                  throw ResolutionError(
                      r.keyword, "Can't return a value from an initializer.");
                }
                resolve(*r.value);
              }
            },
        },
        stmt);
  }

  void resolve(const Declaration &decl) {
    std::visit(overload{
                   [this](const VariableDeclaration &d) -> void {
                     declare(d.name);
                     if (d.initializer) {
                       resolve(*d.initializer);
                     }
                     define(d.name);
                   },
                   [this](const Statement &s) -> void { resolve(s); },
                   [this](const FunctionDeclaration &d) -> void {
                     resolve(d, FunctionType::FUNCTION);
                   },
                   [this](const ClassDeclaration &d) -> void {
                     ClassType enclosing_class = m_current_class;
                     m_current_class = ClassType::CLASS;

                     declare(d.name);
                     define(d.name);

                     if (d.superclass) {
                       m_current_class = ClassType::SUBCLASS;

                       if (d.name.lexeme() == d.superclass->name.lexeme()) {
                         throw ResolutionError(
                             d.superclass->name,
                             "A class can't inherit from itself.");
                       }
                       resolve(Expression{*d.superclass});

                       begin_scope();
                       m_scopes.back()["super"] = true;
                     }

                     for (const auto &method : d.methods) {
                       FunctionType type = FunctionType::METHOD;
                       if (method->name.lexeme() == "init") {
                         type = FunctionType::INITIALIZER;
                       }

                       if (method->is_static) {
                         type = FunctionType::STATIC_METHOD;
                         resolve(*method, type);
                       } else {
                         begin_scope();
                         m_scopes.back()["this"] = true;
                         resolve(*method, type);
                         end_scope();
                       }
                     }

                     if (d.superclass) {
                       end_scope();
                     }

                     m_current_class = enclosing_class;
                   },
               },
               decl);
  }

  void resolve_declarations(const std::vector<Declaration> &declarations) {
    for (const auto &decl : declarations) {
      resolve(decl);
    }
  }

  void resolve_block(const BlockStatement &block) {
    for (const auto &decl : block.declarations) {
      resolve(decl);
    }
  }

public:
  Resolver(interpreter::Interpreter &interpreter)
      : m_interpreter(interpreter) {}

  void resolve(const Program &program) {
    resolve_declarations(program.declarations);
  }
};

} // namespace seam::resolver
