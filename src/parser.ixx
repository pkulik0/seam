module;

#include <any>
#include <format>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module seam.parser;

import seam.token;
import seam.ast;
import seam.common;

using namespace seam::ast;
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

  Program parse() { return parse_program(); }

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

  Token peek_next() {
    if (m_current + 1 >= m_tokens.size())
      return m_tokens.back(); // END_OF_FILE
    return m_tokens.at(m_current + 1);
  }

  Token previous() { return m_tokens.at(m_current - 1); }

  Program parse_program() {
    Program program;
    while (!is_at_end()) {
      auto declaration = parse_declaration();
      if (declaration) {
        program.declarations.emplace_back(std::move(*declaration));
      }
    }
    return program;
  }

  std::optional<Declaration> parse_declaration() {
    try {
      if (match(Type::VAR)) {
        return parse_variable_declaration();
      }
      return parse_statement();
    } catch (const Error &e) {
      std::println("{}", e.what());
      synchronize();
      return std::nullopt;
    }
  }

  VariableDeclaration parse_variable_declaration() {
    const auto name = consume(Type::IDENTIFIER, "Expected variable name.");

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(Type::EQUAL)) {
      initializer = make_expression(parse_expression());
    }

    consume(Type::SEMICOLON, "Expected ';' after variable declaration.");
    return VariableDeclaration{name, std::move(initializer)};
  }

  Statement parse_statement() {
    if (match(Type::WHILE)) {
      return parse_while_statement();
    }
    if(match(Type::FOR)) {
      return parse_for_statement();
    }
    if (match(Type::IF)) {
      return parse_if_statement();
    }
    if (match(Type::PRINT)) {
      return parse_print_statement();
    }
    if (match(Type::LEFT_BRACE)) {
      return parse_block();
    }
    return parse_expression_statement();
  }

  ForStatement parse_for_statement() {
    consume(Type::LEFT_PAREN, "Expected '(' after 'for'.");

    std::unique_ptr<Declaration> initializer;
    if (match(Type::SEMICOLON)) {
      // No initializer
    } else if (match(Type::VAR)) {
      initializer = std::make_unique<Declaration>(parse_variable_declaration());
    } else {
      initializer = std::make_unique<Declaration>(parse_expression());
      consume(Type::SEMICOLON, "Expected ';' after initializer.");
    }

    std::unique_ptr<Expression> condition;
    if (match(Type::SEMICOLON)) {
      condition = make_expression(Literal{true});
    } else {
      condition = make_expression(parse_expression());
      consume(Type::SEMICOLON, "Expected ';' after condition.");
    }

    std::unique_ptr<Expression> increment;
    if (!match(Type::RIGHT_PAREN)) {
      increment = make_expression(parse_expression());
      consume(Type::RIGHT_PAREN, "Expected ')' after for statement.");
    }

    auto body = std::make_unique<Statement>(parse_statement());

    return ForStatement{std::move(initializer),
                        std::move(condition),
                        std::move(increment),
                        std::move(body)};
  }

  WhileStatement parse_while_statement() {
    consume(Type::LEFT_PAREN, "Expected '(' after 'while'.");
    auto condition = parse_expression();
    consume(Type::RIGHT_PAREN, "Expected ')' after condition.");
    return WhileStatement{make_expression(std::move(condition)),
                          std::make_unique<Statement>(parse_statement())};
  }

  IfStatement parse_if_statement() {
    consume(Type::LEFT_PAREN, "Expected '(' after 'if'.");
    auto condition = parse_expression();
    consume(Type::RIGHT_PAREN, "Expected ')' after condition.");
    auto then_branch = std::make_unique<Statement>(parse_statement());

    std::unique_ptr<Statement> else_branch = nullptr;
    if (match(Type::ELSE)) {
      else_branch = std::make_unique<Statement>(parse_statement());
    }

    return IfStatement{make_expression(std::move(condition)),
                       std::move(then_branch), std::move(else_branch)};
  }

  PrintStatement parse_print_statement() {
    auto expr = parse_expression();
    consume(Type::SEMICOLON, "Expected ';' after print statement.");
    return PrintStatement{make_expression(std::move(expr))};
  }

  BlockStatement parse_block() {
    std::vector<Declaration> declarations;

    while (!check(Type::RIGHT_BRACE) && !is_at_end()) {
      auto declaration = parse_declaration();
      if (declaration) {
        declarations.emplace_back(std::move(*declaration));
      }
    }

    consume(Type::RIGHT_BRACE, "Expected '}' after block.");
    return BlockStatement{std::move(declarations)};
  }

  Statement parse_expression_statement() {
    auto expr = parse_expression();
    consume(Type::SEMICOLON, "Expected ';' after expression.");
    return std::move(expr);
  }

  Expression parse_expression() { return parse_assignment(); }

  Expression parse_assignment() {
    if (check(Type::IDENTIFIER) && peek_next().type() == Type::EQUAL) {
      auto name = advance();
      advance(); // consume '='
      auto value = parse_assignment();
      return Assignment{name, make_expression(std::move(value))};
    }

    return parse_ternary();
  }

  Expression parse_ternary() {
    auto condition = parse_or();

    if (!match(Type::QUESTION_MARK)) {
      return condition;
    }

    auto true_expr = parse_ternary();
    consume(Type::COLON, "Expected ':' after ternary expression.");
    auto false_expr = parse_ternary();
    return Ternary{make_expression(std::move(condition)),
                   make_expression(std::move(true_expr)),
                   make_expression(std::move(false_expr))};
  }

  Expression parse_or() {
    auto expr = parse_and();

    while (match(Type::OR)) {
      const auto op = previous();
      auto right = parse_and();
      expr = Logical{make_expression(std::move(expr)), op,
                     make_expression(std::move(right))};
    }

    return expr;
  }

  Expression parse_and() {
    auto expr = parse_equality();

    while (match(Type::AND)) {
      const auto op = previous();
      auto right = parse_equality();
      expr = Logical{make_expression(std::move(expr)), op,
                     make_expression(std::move(right))};
    }

    return expr;
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
      return Literal{false};
    if (match(Type::TRUE))
      return Literal{true};
    if (match(Type::NIL))
      return Literal{std::any{}};

    if (match(Type::NUMBER, Type::STRING)) {
      return Literal{previous().literal().value()};
    }

    if (match(Type::IDENTIFIER)) {
      return Variable{previous()};
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
