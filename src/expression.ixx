module;

#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

export module seam.expression;

import seam.token;

export namespace seam::expression {

struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Ternary;

using Expression = std::variant<Binary, Grouping, Literal, Unary, Ternary>;

struct Binary {
  std::unique_ptr<Expression> left;
  token::Token op;
  std::unique_ptr<Expression> right;
};

struct Grouping {
  std::unique_ptr<Expression> expr;
};

struct Literal {
  std::string_view value;
};

struct Unary {
  token::Token op;
  std::unique_ptr<Expression> right;
};

struct Ternary {
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Expression> true_expr;
  std::unique_ptr<Expression> false_expr;
};

template <typename T>
std::unique_ptr<Expression> make_expression(T&& expr) {
  return std::make_unique<Expression>(std::forward<T>(expr));
}

} // namespace seam::expression
