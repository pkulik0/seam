module;

#include <any>
#include <chrono>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <print>
#include <string>
#include <unordered_map>
#include <variant>

export module seam.interpreter;

import seam.ast;
import seam.common;
import seam.runtime;
import seam.token;
import seam.error;

using namespace seam::ast;
using namespace seam::token;
using namespace seam::runtime;

export namespace seam::interpreter {

class Interpreter {
  std::shared_ptr<Environment> m_globals;
  std::shared_ptr<Environment> m_env;
  std::unordered_map<const Expression *, int> m_locals;
  std::ostream &m_out;

  class ReturnValue : public std::exception {
  public:
    ReturnValue(const std::any &value) : m_value(value) {}
    const std::any &value() const { return m_value; }

  private:
    std::any m_value;
  };

  class ScopeGuard {
    Interpreter &m_interp;
    std::shared_ptr<Environment> m_env;
    std::shared_ptr<Environment> m_previous;

  public:
    ScopeGuard(Interpreter &interp,
               std::shared_ptr<Environment> parent = nullptr)
        : m_interp(interp), m_previous(interp.m_env) {
      m_env = std::make_shared<Environment>(parent ? parent : interp.m_env);
      m_interp.m_env = m_env;
    }
    ~ScopeGuard() { m_interp.m_env = m_previous; }
  };

  class SeamCallable {
    usize m_arity;

  public:
    SeamCallable(usize arity) : m_arity(arity) {}
    virtual ~SeamCallable() = default;
    virtual std::any call(Interpreter &interpreter,
                          std::vector<std::any> &&arguments) = 0;
    virtual usize arity() const { return m_arity; }
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

  class SeamInstance;

  class SeamFunction : public SeamCallable {
    const FunctionDeclaration &m_declaration;
    std::shared_ptr<Environment> m_closure;
    bool m_is_initializer;
    bool m_is_static;

  public:
    SeamFunction(const FunctionDeclaration &declaration,
                 std::shared_ptr<Environment> closure, bool is_initializer)
        : SeamCallable(declaration.parameters.size()),
          m_declaration(declaration), m_closure(closure),
          m_is_initializer(is_initializer), m_is_static(declaration.is_static) {
    }

    bool is_static() const { return m_is_static; }

    std::shared_ptr<SeamCallable> bind(std::shared_ptr<SeamInstance> instance) {
      auto env = std::make_shared<Environment>(m_closure);
      env->define("this", instance);
      return std::static_pointer_cast<SeamCallable>(
          std::make_shared<SeamFunction>(m_declaration, env, m_is_initializer));
    }

    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      ScopeGuard scope(interpreter, m_closure);
      for (const auto &param : m_declaration.parameters) {
        interpreter.m_env->define(param, arguments.front());
        arguments.erase(arguments.begin());
      }
      try {
        interpreter.execute_block(*m_declaration.body);
      } catch (const ReturnValue &return_value) {
        if (m_is_initializer) {
          return m_closure->getAt(0, "this");
        }
        return return_value.value();
      }
      if (m_is_initializer) {
        return m_closure->getAt(0, "this");
      }
      return std::any(std::nullopt);
    }
  };

  class SeamLambda : public SeamCallable {
    const FunctionExpression &m_expression;
    std::shared_ptr<Environment> m_closure;

  public:
    SeamLambda(const FunctionExpression &expression,
               std::shared_ptr<Environment> closure)
        : SeamCallable(expression.parameters.size()), m_expression(expression),
          m_closure(closure) {}

    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      ScopeGuard scope(interpreter, m_closure);
      for (const auto &param : m_expression.parameters) {
        interpreter.m_env->define(param, arguments.front());
        arguments.erase(arguments.begin());
      }
      try {
        interpreter.execute_block(*m_expression.body);
      } catch (const ReturnValue &e) {
        return e.value();
      }
      return std::any(std::nullopt);
    }
  };

  class SeamClass;

  class SeamInstance : public std::enable_shared_from_this<SeamInstance> {
    std::shared_ptr<const SeamClass> m_klass;
    std::unordered_map<std::string, std::any> m_fields;

  public:
    SeamInstance(std::shared_ptr<const SeamClass> klass)
        : m_klass(std::move(klass)) {}

    std::string_view name() const { return m_klass->name(); }

    std::any get(const Token &name) {
      auto it = m_fields.find(std::string{name.lexeme()});
      if (it != m_fields.end()) {
        return it->second;
      }
      auto opt_method = m_klass->get_method(name);
      if (opt_method && !opt_method.value()->is_static()) {
        return std::static_pointer_cast<SeamCallable>(
            opt_method.value()->bind(shared_from_this()));
      }
      throw RuntimeError(
          name, std::format("Undefined property '{}'.", name.lexeme()));
    }

    void set(const Token &name, const std::any &value) {
      m_fields[std::string{name.lexeme()}] = value;
    }
  };

  class SeamClass : public SeamCallable,
                    public std::enable_shared_from_this<SeamClass> {
    Token m_name;
    std::shared_ptr<SeamClass> m_superclass;

  public:
    using MethodMap =
        std::unordered_map<std::string, std::shared_ptr<SeamFunction>>;

  private:
    MethodMap m_methods;

  public:
    SeamClass(const Token &name, std::shared_ptr<SeamClass> superclass,
              MethodMap &&methods)
        : SeamCallable(0), m_name(name), m_superclass(std::move(superclass)),
          m_methods(std::move(methods)) {}

    std::any call(Interpreter &interpreter,
                  std::vector<std::any> &&arguments) override {
      auto instance = std::make_shared<SeamInstance>(shared_from_this());
      const auto opt_init = get_method("init");
      if (opt_init) {
        opt_init.value()->bind(instance)->call(interpreter,
                                               std::move(arguments));
      }
      return instance;
    }

    std::string_view name() const { return m_name.lexeme(); }

    std::optional<std::shared_ptr<SeamFunction>>
    get_method(const Token &name) const {
      return get_method(std::string{name.lexeme()});
    }

    std::optional<std::shared_ptr<SeamFunction>>
    get_method(const std::string &name) const {
      auto it = m_methods.find(name);
      if (it != m_methods.end()) {
        return it->second;
      }
      if (m_superclass) {
        return m_superclass->get_method(name);
      }
      return std::nullopt;
    }

    usize arity() const override {
      const auto opt_init = get_method("init");
      return opt_init ? opt_init.value()->arity() : 0;
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
    if (value.type() == typeid(std::shared_ptr<SeamInstance>)) {
      auto instance = std::any_cast<std::shared_ptr<SeamInstance>>(value);
      return std::format("{} instance", instance->name());
    }
    if (value.type() == typeid(std::shared_ptr<SeamCallable>)) {
      auto callable = std::any_cast<std::shared_ptr<SeamCallable>>(value);
      if (auto klass = std::dynamic_pointer_cast<SeamClass>(callable)) {
        return std::string(klass->name());
      }
      return "function";
    }
    return std::string(value.type().name());
  }

  std::any evaluate(const Expression &expr) {
    const Expression *expr_ptr = &expr;
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
            [this, expr_ptr](const Variable &e) -> std::any {
              return lookUpVariable(e.name, expr_ptr);
            },
            [this, expr_ptr](const Assignment &e) -> std::any {
              const auto value = evaluate(*e.value);

              const auto it = m_locals.find(expr_ptr);
              if (it != m_locals.end()) {
                m_env->assignAt(it->second, e.name, value);
              } else {
                m_globals->assign(e.name, value);
              }
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
            [this](const Get &e) -> std::any {
              std::any object = evaluate(*e.object);
              if (object.type() == typeid(std::shared_ptr<SeamInstance>)) {
                auto instance =
                    std::any_cast<std::shared_ptr<SeamInstance>>(object);
                return instance->get(e.name);
              }

              if (object.type() == typeid(std::shared_ptr<SeamCallable>)) {
                auto callable =
                    std::any_cast<std::shared_ptr<SeamCallable>>(object);
                auto klass = std::dynamic_pointer_cast<SeamClass>(callable);
                if (klass) {
                  auto opt_method = klass->get_method(e.name);
                  if (opt_method && opt_method.value()->is_static()) {
                    return std::static_pointer_cast<SeamCallable>(
                        opt_method.value());
                  }
                }
              }

              throw RuntimeError(e.name,
                                 "Only instances and classes have properties.");
            },
            [this](const Set &e) -> std::any {
              std::any value = evaluate(*e.value);
              std::any object = evaluate(*e.object);
              if (object.type() != typeid(std::shared_ptr<SeamInstance>)) {
                throw RuntimeError(e.name, "Only instances have properties.");
              }

              auto instance =
                  std::any_cast<std::shared_ptr<SeamInstance>>(object);
              instance->set(e.name, value);

              return value;
            },
            [this, expr_ptr](const ThisExpr &e) -> std::any {
              return lookUpVariable(e.keyword, expr_ptr);
            },
            [this, expr_ptr](const Super &s) -> std::any {
              auto it = m_locals.find(expr_ptr);
              if(it == m_locals.end()) {
                throw RuntimeError(s.keyword, "Super not available in this context.");
              }
              auto distance = it->second;
              auto opt_superclass = m_env->ancestor(distance)->get("super");
              if (!opt_superclass) {
                throw RuntimeError(s.keyword, "Superclass not initialized.");
              }
              std::shared_ptr<SeamCallable> super = std::any_cast<std::shared_ptr<SeamCallable>>(*opt_superclass);
              std::shared_ptr<SeamClass> superclass = std::dynamic_pointer_cast<SeamClass>(super);
              if (!superclass) {
                throw RuntimeError(s.keyword, "Superclass not initialized.");
              }
              auto opt_method = superclass->get_method(s.method);
              if (!opt_method) {
                throw RuntimeError(s.method, std::format("Undefined property '{}'.", s.method.lexeme()));
              }
              
              auto instance = std::any_cast<std::shared_ptr<SeamInstance>>(
                  m_env->getAt(distance - 1, "this"));
              return opt_method.value()->bind(instance);
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
    if (callable->arity() != arguments.size()) {
      throw RuntimeError(paren,
                         std::format("Expected {} arguments but got {}.",
                                     callable->arity(), arguments.size()));
    }
    return callable->call(*this, std::move(arguments));
  }

  void execute_block(const BlockStatement &block) {
    for (const auto &decl : block.declarations) {
      execute_declaration(decl);
    }
  }

  void execute_statement(const Statement &statement) {
    std::visit(
        overload{
            [this](const PrintStatement &s) -> void {
              std::println(m_out, "{}", stringify(evaluate(*s.expression)));
            },
            [this](const Expression &e) -> void { evaluate(e); },
            [this](const BlockStatement &b) -> void {
              ScopeGuard scope(*this);
              execute_block(b);
            },
            [this](const IfStatement &i) -> void {
              if (is_truthy(evaluate(*i.condition))) {
                execute_statement(*i.then_branch);
              } else if (i.else_branch) {
                execute_statement(*i.else_branch);
              }
            },
            [this](const WhileStatement &w) -> void {
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
              std::any value =
                  r.value ? evaluate(*r.value) : std::any(std::nullopt);
              throw ReturnValue(value);
            },
        },
        statement);
  }

  void execute_declaration(const Declaration &declaration) {
    std::visit(
        overload{
            [this](const VariableDeclaration &d) -> void {
              auto value = evaluate(*d.initializer);
              m_env->define(d.name, value);
            },
            [this](const Statement &statement) -> void {
              execute_statement(statement);
            },
            [this](const FunctionDeclaration &d) -> void {
              std::shared_ptr<SeamCallable> function =
                  std::make_shared<SeamFunction>(d, m_env, false);
              m_env->define(d.name, function);
            },
            [this](const ClassDeclaration &d) -> void {
              m_env->define(d.name, std::nullopt);

              std::shared_ptr<SeamClass> superclass_ptr = nullptr;
              if (d.superclass) {
                auto superclass_value =
                    lookUpVariable(d.superclass->name, nullptr);
                if (superclass_value.type() !=
                    typeid(std::shared_ptr<SeamCallable>)) {
                  throw RuntimeError(d.superclass->name,
                                     "Superclass must be a class.");
                }
                auto superclass_callable =
                    std::any_cast<std::shared_ptr<SeamCallable>>(
                        superclass_value);
                superclass_ptr =
                    std::dynamic_pointer_cast<SeamClass>(superclass_callable);
                if (!superclass_ptr) {
                  throw RuntimeError(d.superclass->name,
                                     "Superclass must be a class.");
                }

                m_env = std::make_shared<Environment>(m_env);
                m_env->define("super", std::static_pointer_cast<SeamCallable>(
                                           superclass_ptr));
              }

              SeamClass::MethodMap methods;
              for (const auto &method : d.methods) {
                const auto name = std::string{method->name.lexeme()};
                const bool is_initializer = name == "init";
                methods[name] = std::make_shared<SeamFunction>(*method, m_env,
                                                               is_initializer);
              }

              std::shared_ptr<SeamCallable> class_ =
                  std::make_shared<SeamClass>(d.name, std::move(superclass_ptr),
                                              std::move(methods));

              if (d.superclass) {
                m_env = m_env->enclosing();
              }
              m_env->assign(d.name, class_);
            },
        },
        declaration);
  }

  std::any lookUpVariable(const Token &name, const Expression *expr) {
    const auto it = m_locals.find(expr);
    if (it != m_locals.end()) {
      return m_env->getAt(it->second, std::string{name.lexeme()});
    } else {
      const auto value = m_globals->get(std::string{name.lexeme()});
      if (value) {
        return *value;
      }
      throw RuntimeError(
          name, std::format("Undefined variable '{}'.", name.lexeme()));
    }
  }

public:
  void resolve(const Expression *expr, int depth) { m_locals[expr] = depth; }

  Interpreter(std::ostream &out = std::cout)
      : m_globals(std::make_shared<Environment>()), m_env(m_globals),
        m_out(out) {
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

  void execute(const Program &program) {
    for (const auto &declaration : program.declarations) {
      execute_declaration(declaration);
    }
  }
};

} // namespace seam::interpreter
