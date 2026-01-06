module;

#include <any>
#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <print>
#include <string>
#include <unordered_map>
#include <variant>

export module seam.interpreter;

import seam.ast;
import seam.common;
import seam.token;

using namespace seam::ast;
using namespace seam::token;

export namespace seam::interpreter {

template <typename... Ts> struct overload : Ts... {
  using Ts::operator()...;
};

class ReturnValue : public std::exception {
public:
  ReturnValue(const std::any &value) : m_value(value) {}
  const std::any &value() const { return m_value; }
private:
  std::any m_value;
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
  Environment(std::shared_ptr<Environment> parent) : m_enclosing(parent) {}

  void define(const Token &name, const std::any &value) {
    m_values[std::string{name.lexeme()}] = value;
  }

  void define(const std::string &name, const std::any &value) {
    m_values[name] = value;
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
  std::shared_ptr<Environment> m_enclosing = nullptr;
};

class Interpreter {
private:
  std::shared_ptr<Environment> m_globals;
  std::shared_ptr<Environment> m_env;

  class ScopeGuard {
  public:
    ScopeGuard(Interpreter &interp, std::shared_ptr<Environment> parent = nullptr)
        : m_interp(interp), m_previous(interp.m_env) {
      m_env = std::make_shared<Environment>(parent ? parent : interp.m_env);
      m_interp.m_env = m_env;
    }
    ~ScopeGuard() { m_interp.m_env = m_previous; }

  private:
    Interpreter &m_interp;
    std::shared_ptr<Environment> m_env;
    std::shared_ptr<Environment> m_previous;
  };

  struct SeamCallable {
    SeamCallable(usize arity) : arity(arity) {}
    virtual ~SeamCallable() = default;
    virtual std::any call(Interpreter &interpreter,
                          std::vector<std::any> &&arguments) = 0;
    usize arity;
  };

  struct SeamNativeFunction : public SeamCallable {
    using Function =
        std::function<std::any(Interpreter &, std::vector<std::any> &&)>;
    Function function;
    SeamNativeFunction(Function function, usize arity)
        : SeamCallable(arity), function(std::move(function)) {}
    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      return function(interpreter, std::move(arguments));
    }
  };

  struct SeamFunction : public SeamCallable {
    const FunctionDeclaration &declaration;
    std::shared_ptr<Environment> closure;

    SeamFunction(const FunctionDeclaration &declaration, std::shared_ptr<Environment> closure)
        : SeamCallable(declaration.parameters.size()),
          declaration(declaration), closure(closure) {}
    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      ScopeGuard scope(interpreter, closure);
      for (const auto &param : declaration.parameters) {
        interpreter.m_env->define(param, arguments.front());
        arguments.erase(arguments.begin());
      }
      try {
        interpreter.execute_block(*declaration.body);
      } catch (const ReturnValue &e) {
        return e.value();
      }
      return std::any(std::nullopt);
    }
  };

  struct SeamLambda : public SeamCallable {
    const FunctionExpression &expression;
    std::shared_ptr<Environment> closure;

    SeamLambda(const FunctionExpression &expression, std::shared_ptr<Environment> closure)
        : SeamCallable(expression.parameters.size()),
          expression(expression), closure(closure) {}
    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      ScopeGuard scope(interpreter, closure);
      for (const auto &param : expression.parameters) {
        interpreter.m_env->define(param, arguments.front());
        arguments.erase(arguments.begin());
      }
      try {
        interpreter.execute_block(*expression.body);
      } catch (const ReturnValue &e) {
        return e.value();
      }
      return std::any(std::nullopt);
    }
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
                if (is_truthy(left))
                  return left;
              } else {
                if (!is_truthy(left))
                  return left;
              }

              return evaluate(*e.right);
            },
            [this](const Call &e) -> std::any {
              std::any callee = evaluate(*e.callee);
              std::vector<std::any> arguments{};
              arguments.reserve(e.arguments.size());
              for (const auto &arg : e.arguments) {
                arguments.emplace_back(evaluate(*arg));
              }
              return call(callee, std::move(arguments), e.paren);
            },
            [this](const FunctionExpression &e) -> std::any {
              return std::static_pointer_cast<SeamCallable>(
                  std::make_shared<SeamLambda>(e, m_env));
            },
        },
        expr);
  }

  std::any call(const std::any &callee, std::vector<std::any> arguments,
                const Token &paren) {
    if (callee.type() != typeid(std::shared_ptr<SeamCallable>)) {
      throw RuntimeError(paren, "Can only call functions and classes.");
    }
    auto callable = std::any_cast<std::shared_ptr<SeamCallable>>(callee);
    if (callable->arity != arguments.size()) {
      throw RuntimeError(paren, std::format("Expected {} arguments but got {}.",
                                            callable->arity, arguments.size()));
    }
    return callable->call(*this, std::move(arguments));
  }

  void execute_block(const BlockStatement &block) {
    ScopeGuard scope(*this);
    for (const auto &decl : block.declarations) {
      execute_declaration(decl);
    }
  }

  void execute_statement(const Statement &statement) {
    std::visit(overload{
                   [this](const PrintStatement &s) -> void {
                     std::println("{}", stringify(evaluate(*s.expression)));
                   },
                   [this](const Expression &e) -> void { evaluate(e); },
                   [this](const BlockStatement &b) -> void {
                     execute_block(b);
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
                   [this](const ReturnStatement &r) -> void {
                     std::any value = r.value ? evaluate(*r.value) : std::any(std::nullopt);
                     throw ReturnValue(value);
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
                   [this](const FunctionDeclaration &d) -> void {
                     std::shared_ptr<SeamCallable> function =
                         std::make_shared<SeamFunction>(d, m_env);
                     m_env->define(d.name, function);
                   },
               },
               declaration);
  }

public:
  Interpreter() : m_globals(std::make_shared<Environment>()), m_env(m_globals) {
    std::shared_ptr<SeamCallable> clock = std::make_shared<SeamNativeFunction>(
        [](Interpreter &, std::vector<std::any> &&) -> std::any {
          using namespace std::chrono;
          double time = duration_cast<milliseconds>(
                            system_clock::now().time_since_epoch())
                            .count();
          return std::any(time);
        },
        0);
    m_globals->define("clock", clock);
  }

  void run(const Program &program) {
    for (const auto &declaration : program.declarations) {
      execute_declaration(declaration);
    }
  }
};

} // namespace seam::interpreter
