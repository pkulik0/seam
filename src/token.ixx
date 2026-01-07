module;

#include <any>
#include <format>
#include <optional>
#include <string>
#include <string_view>

#include <magic_enum/magic_enum.hpp>

export module seam.token;

import seam.common;

export namespace seam::token {

enum class Type {
  // Single-character tokens
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  QUESTION_MARK,
  COLON,
  // One or two character tokens
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,

  // Literals
  COMMENT,
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,

  END_OF_FILE,
};

class Token {
public:
  Token(Type type, std::string_view lexeme,
        std::optional<std::any> literal, usize line, usize column)
      : m_type(type), m_lexeme(lexeme), m_literal(literal), m_line(line),
        m_column(column) {}

  std::string to_string() const {
    return std::format("{} {} ({}:{})", magic_enum::enum_name(m_type), m_lexeme, m_line, m_column);
  }

  Type type() const { return m_type; }

  std::string_view lexeme() const { return m_lexeme; }

  const std::optional<std::any>& literal() const { return m_literal; }

  usize line() const { return m_line; }

  usize column() const { return m_column; }


private:
  Type m_type;
  std::string m_lexeme;
  std::optional<std::any> m_literal;
  usize m_line = 0;
  usize m_column = 0;
};

} // namespace seam::token