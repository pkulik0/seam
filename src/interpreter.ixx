module;

#include <any>
#include <format>
#include <memory>
#include <print>
#include <string>
#include <unordered_map>
#include <variant>

export module seam.interpreter;

import seam.ast;
import seam.token;

using namespace seam::ast;
using namespace seam::token;

export namespace seam::interpreter {

template <typename... Ts> struct overload : Ts... {
  using Ts::operator()...;
};

class RuntimeError : public std::exception {
public:
  RuntimeError(const Token &token, const std::string_view message)
      : m_message(std::format("Runtime error: {} ({}:{})", message,
                              token.line(), token.column())) {}
  const char *what() const noexcept override { return m_message.c_str(); }

private:
  std::string m_message;
};

class Environment {
public:
  Environment() = default;
  Environment(Environment *parent) : m_enclosing(parent) {}

  void define(const Token &name, const std::any &value) {
    m_values[std::string{name.lexeme()}] = value;
  }

  void assign(const Token &name, const std::any &value) {
    const auto it = m_values.find(std::string{name.lexeme()});
    if (it != m_values.end()) {
      it->second = value;
      return;
    }
    if (m_enclosing) {
      m_enclosing->assign(name, value);
      return;
    }
    throw RuntimeError(name,
                       std::format("Undefined variable '{}'.", name.lexeme()));
  }

  std::optional<std::any> get(const std::string &name) const {
    const auto it = m_values.find(name);
    if (it != m_values.end()) {
      return it->second;
    }
    if (m_enclosing) {
      return m_enclosing->get(name);
    }
    return std::nullopt;
  }

private:
  std::unordered_map<std::string, std::any> m_values{};
  Environment *m_enclosing = nullptr;
};

class Interpreter {
private:
  Environment m_globals;
  Environment *m_env = &m_globals;

  class ScopeGuard {
  public:
    ScopeGuard(Interpreter &interp)
        : m_interp(interp), m_env(interp.m_env), m_previous(interp.m_env) {
      m_interp.m_env = &m_env;
    }
    ~ScopeGuard() { m_interp.m_env = m_previous; }

  private:
    Interpreter &m_interp;
    Environment m_env;
    Environment *m_previous;
  };

  bool is_truthy(const std::any &value) const {
    if (!value.has_value()) {
      return false;
    }
    if (value.type() == typeid(bool)) {
      return std::any_cast<bool>(value);
    }
    if (value.type() == typeid(double)) {
      return std::any_cast<double>(value) != 0;
    }
    if (value.type() == typeid(std::string_view)) {
      return !std::any_cast<std::string_view>(value).empty();
    }
    if (value.type() == typeid(std::string)) {
      return !std::any_cast<std::string>(value).empty();
    }
    throw std::runtime_error("Unknown type");
  }

  bool is_equal(const std::any &a, const std::any &b) const {
    if (a.type() != b.type()) {
      return false;
    }
    if (a.type() == typeid(double)) {
      return std::any_cast<double>(a) == std::any_cast<double>(b);
    }
    if (a.type() == typeid(bool)) {
      return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    }
    if (a.type() == typeid(std::string_view)) {
      return std::any_cast<std::string_view>(a) ==
             std::any_cast<std::string_view>(b);
    }
    if (a.type() == typeid(std::string)) {
      return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
    }
    if (!a.has_value() && !b.has_value()) {
      return true;
    }
    return false;
  }

  void check_number_operand(const Token &op, const std::any &operand) const {
    if (operand.type() == typeid(double)) {
      return;
    }
    throw RuntimeError(op, "Operand must be a number.");
  }

  void check_number_operands(const Token &op, const std::any &left,
                             const std::any &right) const {
    check_number_operand(op, left);
    check_number_operand(op, right);
  }

  std::string get_string(const std::any &value) const {
    if (value.type() == typeid(std::string)) {
      return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(std::string_view)) {
      return std::string(std::any_cast<std::string_view>(value));
    }
    if (value.type() == typeid(double)) {
      return std::format("{}", std::any_cast<double>(value));
    }
    return "";
  }

  std::string stringify(const std::any &value) const {
    if (!value.has_value()) {
      return "nil";
    }
    if (value.type() == typeid(double)) {
      return std::format("{}", std::any_cast<double>(value));
    }
    if (value.type() == typeid(bool)) {
      return std::any_cast<bool>(value) ? "true" : "false";
    }
    if (value.type() == typeid(std::string_view)) {
      return std::string(std::any_cast<std::string_view>(value));
    }
    if (value.type() == typeid(std::string)) {
      return std::any_cast<std::string>(value);
    }
    return std::string(value.type().name());
  }

  std::any evaluate(const Expression &expr) {
    return std::visit(
        overload{
            [this](const Binary &e) -> std::any {
              std::any left = evaluate(*e.left);
              std::any right = evaluate(*e.right);

              switch (e.op.type()) {
              case Type::MINUS:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) -
                       std::any_cast<double>(right);
              case Type::STAR:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) *
                       std::any_cast<double>(right);
              case Type::SLASH:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) /
                       std::any_cast<double>(right);
              case Type::PLUS: {
                if (left.type() == typeid(double) &&
                    right.type() == typeid(double)) {
                  return std::any_cast<double>(left) +
                         std::any_cast<double>(right);
                }

                const auto is_left_string =
                    left.type() == typeid(std::string_view) ||
                    left.type() == typeid(std::string);
                const auto is_right_string =
                    right.type() == typeid(std::string_view) ||
                    right.type() == typeid(std::string);
                if (is_left_string || is_right_string) {
                  return get_string(left) + get_string(right);
                }
                throw RuntimeError(
                    e.op, "Operands must be two numbers or two strings.");
              }
              case Type::GREATER:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) >
                       std::any_cast<double>(right);
              case Type::GREATER_EQUAL:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) >=
                       std::any_cast<double>(right);
              case Type::LESS:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) <
                       std::any_cast<double>(right);
              case Type::LESS_EQUAL:
                check_number_operands(e.op, left, right);
                return std::any_cast<double>(left) <=
                       std::any_cast<double>(right);
              case Type::BANG_EQUAL:
                return !is_equal(left, right);
              case Type::EQUAL_EQUAL:
                return is_equal(left, right);
              default:
                throw RuntimeError(e.op, "Unknown operator");
              }
            },
            [this](const Grouping &e) -> std::any { return evaluate(*e.expr); },
            [](const Literal &e) -> std::any { return e.value; },
            [this](const Unary &e) -> std::any {
              std::any right = evaluate(*e.right);

              switch (e.op.type()) {
              case Type::MINUS:
                check_number_operand(e.op, right);
                return std::any_cast<double>(right) * -1;
              case Type::BANG:
                return !is_truthy(right);
              default:
                // Unreachable
                throw RuntimeError(e.op, "Unknown operator");
              }
            },
            [this](const Ternary &e) -> std::any {
              std::any condition = evaluate(*e.condition);
              if (is_truthy(condition)) {
                return evaluate(*e.true_expr);
              } else {
                return evaluate(*e.false_expr);
              }
            },
            [this](const Variable &e) -> std::any {
              const auto value = m_env->get(std::string{e.name.lexeme()});
              if (value) {
                return *value;
              }
              throw RuntimeError(e.name, std::format("Undefined variable '{}'.",
                                                     e.name.lexeme()));
            },
            [this](const Assignment &e) -> std::any {
              const auto value = evaluate(*e.value);
              m_env->assign(e.name, value);
              return value;
            },
            [this](const Logical &e) -> std::any {
              std::any left = evaluate(*e.left);
              
              if (e.op.type() == Type::OR) {
                if (is_truthy(left)) return left;
              } else {
                if (!is_truthy(left)) return left;
              }
              
              return evaluate(*e.right);
            },
        },
        expr);
  }

  void execute_statement(const Statement &statement) {
    std::visit(overload{
                   [this](const PrintStatement &s) -> void {
                     std::println("{}", stringify(evaluate(*s.expression)));
                   },
                   [this](const Expression &e) -> void { evaluate(e); },
                   [this](const BlockStatement &b) -> void {
                     ScopeGuard scope(*this);
                     for (const auto &decl : b.declarations) {
                       execute_declaration(decl);
                     }
                   },
                   [this](const IfStatement &i) -> void {
                     ScopeGuard scope(*this);
                     if (is_truthy(evaluate(*i.condition))) {
                       execute_statement(*i.then_branch);
                     } else if (i.else_branch) {
                       execute_statement(*i.else_branch);
                     }
                   },
                   [this](const WhileStatement &w) -> void {
                     ScopeGuard scope(*this);
                     while (is_truthy(evaluate(*w.condition))) {
                       execute_statement(*w.body);
                     }
                   },
                   [this](const ForStatement &f) -> void {
                     ScopeGuard scope(*this);
                     if (f.initializer) {
                       execute_declaration(*f.initializer);
                     }
                     while (is_truthy(evaluate(*f.condition))) {
                       execute_statement(*f.body);
                       if (f.increment) {
                         evaluate(*f.increment);
                       }
                     }
                   },
               },
               statement);
  }

  void execute_declaration(const Declaration &declaration) {
    std::visit(overload{
                   [this](const VariableDeclaration &d) -> void {
                     auto value = evaluate(*d.initializer);
                     m_env->define(d.name, value);
                   },
                   [this](const Statement &statement) -> void {
                     execute_statement(statement);
                   },
               },
               declaration);
  }

public:
  void run(const Program &program) {
    for (const auto &declaration : program.declarations) {
      execute_declaration(declaration);
    }
  }
};

} // namespace seam::interpreter
