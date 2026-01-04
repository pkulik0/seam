module;

#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module seam.parser;

import seam.token;
import seam.expression;
import seam.common;

using namespace seam::expression;
using namespace seam::token;

export namespace seam::parser {

class Error : public std::exception {
public:
  Error(const Token &token, const std::string_view message) {
    const auto location =
        token.type() == Type::END_OF_FILE
            ? "end of file"
            : std::format("{}:{}", token.line(), token.column());
    m_message = std::format("Parser error: {} ({})", message, location);
  }

  const char *what() const noexcept override { return m_message.c_str(); }

private:
  std::string m_message;
};

class Parser {
public:
  Parser(const std::vector<Token> &tokens) : m_tokens(tokens) {}

  Expression parse() { return parse_expression(); }

private:
  const std::vector<Token> &m_tokens;
  usize m_current = 0;

  template <typename... T> bool match(T... types) {
    for (const auto type : {types...}) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }

  Token consume(Type type, const std::string_view message) {
    if (check(type))
      return advance();
    throw Error(peek(), message);
  }

  void synchronize() {
    advance();

    while (!is_at_end()) {
      if (previous().type() == Type::SEMICOLON)
        return;

      switch (peek().type()) {
      case Type::CLASS:
      case Type::FUN:
      case Type::VAR:
      case Type::FOR:
      case Type::IF:
      case Type::PRINT:
      case Type::RETURN:
      case Type::WHILE:
        return;
      default:
        advance();
      }
    }
  }

  bool check(Type type) {
    if (is_at_end())
      return false;
    return peek().type() == type;
  }

  Token advance() {
    if (!is_at_end())
      m_current++;
    return previous();
  }

  bool is_at_end() { return peek().type() == Type::END_OF_FILE; }

  Token peek() { return m_tokens.at(m_current); }

  Token previous() { return m_tokens.at(m_current - 1); }

  Expression parse_expression() { return parse_ternary(); }

  Expression parse_ternary() {
    auto condition = parse_equality();

    if (!match(Type::QUESTION_MARK)) {
      return condition;
    }

    auto true_expr = parse_equality();
    consume(Type::COLON, "Expected ':' after ternary expression.");
    auto false_expr = parse_equality();
    return Ternary{make_expression(std::move(condition)),
                   make_expression(std::move(true_expr)),
                   make_expression(std::move(false_expr))};
  }

  Expression parse_equality() {
    auto expr = parse_comparison();

    while (match(Type::BANG_EQUAL, Type::EQUAL_EQUAL)) {
      const auto op = previous();
      expr = Binary{make_expression(std::move(expr)), op,
                    make_expression(parse_comparison())};
    }

    return expr;
  }

  Expression parse_comparison() {
    auto expr = parse_term();

    while (match(Type::GREATER, Type::GREATER_EQUAL, Type::LESS,
                 Type::LESS_EQUAL)) {
      const auto op = previous();
      expr = Binary{make_expression(std::move(expr)), op,
                    make_expression(parse_term())};
    }

    return expr;
  }

  Expression parse_term() {
    auto expr = parse_factor();

    while (match(Type::MINUS, Type::PLUS)) {
      const auto op = previous();
      expr = Binary{make_expression(std::move(expr)), op,
                    make_expression(parse_factor())};
    }

    return expr;
  }

  Expression parse_factor() {
    auto expr = parse_unary();

    while (match(Type::SLASH, Type::STAR)) {
      const auto op = previous();
      expr = Binary{make_expression(std::move(expr)), op,
                    make_expression(parse_unary())};
    }

    return expr;
  }

  Expression parse_unary() {
    if (match(Type::BANG, Type::MINUS)) {
      const auto op = previous();
      return Unary{op, make_expression(parse_unary())};
    }

    return parse_primary();
  }

  Expression parse_primary() {
    if (match(Type::FALSE))
      return Literal{"false"};
    if (match(Type::TRUE))
      return Literal{"true"};
    if (match(Type::NIL))
      return Literal{"nil"};

    if (match(Type::NUMBER, Type::STRING)) {
      return Literal{previous().lexeme()};
    }

    if (match(Type::LEFT_PAREN)) {
      auto expr = parse_expression();
      consume(Type::RIGHT_PAREN, "Expected ')' after expression.");
      return Grouping{make_expression(std::move(expr))};
    }

    throw Error(peek(), "Expected expression.");
  }
};

} // namespace seam::parser
