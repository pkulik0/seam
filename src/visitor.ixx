module;

#include <format>
#include <string>
#include <variant>

export module seam.visitor;

import seam.expression;

export namespace seam::visitor {

template <typename... Ts> struct overload : Ts... {
  using Ts::operator()...;
};

class AstPrinter {
public:
  std::string print(const expression::Expression &expr) const {
    return std::visit(overload{
                          [this](const expression::Binary &e) -> std::string {
                            return parenthesize(e.op.lexeme(), *e.left, *e.right);
                          },
                          [this](const expression::Grouping &e) -> std::string {
                            return parenthesize("group", *e.expr);
                          },
                          [](const expression::Literal &e) -> std::string {
                            return e.value.empty() ? "nil"
                                                   : std::string(e.value);
                          },
                          [this](const expression::Unary &e) -> std::string {
                            return parenthesize(e.op.lexeme(), *e.right);
                          },
                          [this](const expression::Ternary &e) -> std::string {
                            return parenthesize("?:", *e.condition, *e.true_expr, *e.false_expr);
                          },
                      },
                      expr);
  }

private:
  template <typename... Exprs>
  std::string parenthesize(std::string_view name, const Exprs &...exprs) const {
    std::string result = std::format("({}", name);
    ((result += std::format(" {}", print(exprs))), ...);
    return result + ")";
  }
};

} // namespace seam::visitor
