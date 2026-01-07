module;

#include <any>
#include <format>
#include <unordered_map>

export module seam.scanner;

import seam.common;
import seam.token;
import seam.error;

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_alphanumeric(char c) { return is_digit(c) || is_alpha(c); }

export namespace seam::scanner {

class ScannerError : public Error {
public:
  ScannerError(usize line, usize column, std::string message)
      : m_message(std::move(message)), m_location({static_cast<int>(line), static_cast<int>(column)}) {}

  [[nodiscard]] auto name() const noexcept -> std::string_view override {
    return "Scanner Error";
  }
  [[nodiscard]] auto reason() const noexcept -> std::string_view override {
    return m_message;
  }
  [[nodiscard]] auto location() const noexcept -> Location override {
	return m_location;
  }

private:
  std::string m_message;
  Location m_location;
};

class Scanner {
public:
  Scanner(const std::string_view source) : m_source(source) {}

  std::vector<token::Token> scan_tokens() {
    while (!is_at_end()) {
      m_start = m_current;
      m_token_start_line = m_line;
      m_token_start_column = m_column + 1;
      scan_token();
    }

    m_tokens.emplace_back(token::Type::END_OF_FILE, "", std::nullopt, m_line,
                          m_column);
    return m_tokens;
  }

private:
  std::string_view m_source;
  std::vector<token::Token> m_tokens{};

  const inline static std::unordered_map<std::string_view, token::Type>
      keywords = {
          {"and", token::Type::AND},       {"struct", token::Type::STRUCT},
          {"else", token::Type::ELSE},     {"false", token::Type::FALSE},
          {"fn", token::Type::FN},         {"for", token::Type::FOR},
          {"if", token::Type::IF},         {"nil", token::Type::NIL},
          {"or", token::Type::OR},         {"print", token::Type::PRINT},
          {"return", token::Type::RETURN}, {"static", token::Type::STATIC}, {"parent", token::Type::PARENT},
          {"self", token::Type::SELF},     {"true", token::Type::TRUE},
          {"let", token::Type::LET},       {"while", token::Type::WHILE},
  };

  usize m_start = 0;
  usize m_current = 0;

  usize m_line = 1;
  usize m_column = 0;
  usize m_token_start_line = 1;
  usize m_token_start_column = 0;

  bool is_at_end() const { return m_current >= m_source.length(); }

  void add_token(token::Type type) {
    m_tokens.emplace_back(type, m_source.substr(m_start, m_current - m_start),
                          std::nullopt, m_token_start_line,
                          m_token_start_column);
  }

  void add_token(token::Type type, std::any literal) {
    m_tokens.emplace_back(type, m_source.substr(m_start, m_current - m_start),
                          literal, m_token_start_line, m_token_start_column);
  }

  void scan_token() {
    const char c = advance();
    switch (c) {
    case '(':
      add_token(token::Type::LEFT_PAREN);
      break;
    case ')':
      add_token(token::Type::RIGHT_PAREN);
      break;
    case '{':
      add_token(token::Type::LEFT_BRACE);
      break;
    case '}':
      add_token(token::Type::RIGHT_BRACE);
      break;
    case ',':
      add_token(token::Type::COMMA);
      break;
    case '.':
      add_token(token::Type::DOT);
      break;
    case '-':
      add_token(token::Type::MINUS);
      break;
    case '+':
      add_token(token::Type::PLUS);
      break;
    case ';':
      add_token(token::Type::SEMICOLON);
      break;
    case '*':
      add_token(token::Type::STAR);
      break;
    case '?':
      add_token(token::Type::QUESTION_MARK);
      break;
    case ':':
      add_token(token::Type::COLON);
      break;
    case '!':
      match('=') ? add_token(token::Type::BANG_EQUAL)
                 : add_token(token::Type::BANG);
      break;
    case '=':
      match('=') ? add_token(token::Type::EQUAL_EQUAL)
                 : add_token(token::Type::EQUAL);
      break;
    case '>':
      match('=') ? add_token(token::Type::GREATER_EQUAL)
                 : add_token(token::Type::GREATER);
      break;
    case '<':
      match('=') ? add_token(token::Type::LESS_EQUAL)
                 : add_token(token::Type::LESS);
      break;
    case '/':
      if (match('/')) {
        scan_comment();
      } else if (match('*')) {
        scan_multiline_comment();
      } else {
        add_token(token::Type::SLASH);
      }
      break;
    case ' ':
    case '\r':
    case '\t':
    case '\n': // Newline is handled by the advance() function
      break;
    case '"':
      scan_string();
      break;
    default:
      if (is_digit(c)) {
        scan_number();
      } else if (is_alpha(c)) {
        scan_identifier();
      } else {
        throw ScannerError(m_line, m_column,
                    std::format("Unexpected character: '{}'", c));
      }
      break;
    }
  }

  char advance() {
    const char c = m_source.at(m_current++);
    m_column++;
    if (c == '\n') {
      m_line++;
      m_column = 0;
    }
    return c;
  }

  bool match(char expected) {
    if (is_at_end())
      return false;
    if (m_source[m_current] != expected)
      return false;
    m_current++;
    if (expected == '\n') {
      m_line++;
      m_column = 0;
    } else {
      m_column++;
    }
    return true;
  }

  char peek() const {
    if (is_at_end())
      return '\0';
    return m_source[m_current];
  }

  char peek_next() const {
    if (m_current + 1 >= m_source.length())
      return '\0';
    return m_source[m_current + 1];
  }

  void scan_comment() {
    while (peek() != '\n' && !is_at_end())
      advance();
    const std::string_view value =
        m_source.substr(m_start + 2, m_current - m_start - 2);
    add_token(token::Type::COMMENT, value);
  }

  void scan_multiline_comment() {
    usize depth = 1;

    while (depth > 0) {
      if (is_at_end())
        throw ScannerError(m_line, m_column, "Unterminated comment.");

      if (peek() == '*' && peek_next() == '/') {
        depth--;
        advance();
      } else if (peek() == '/' && peek_next() == '*') {
        depth++;
        advance();
      }
      advance();
    }

    const std::string_view value =
        m_source.substr(m_start + 2, m_current - m_start - 4);
    add_token(token::Type::COMMENT, value);
  }

  void scan_string() {
    while (peek() != '"' && !is_at_end())
      advance();
    if (is_at_end())
      throw ScannerError(m_line, m_column, "Unterminated string.");

    advance(); // closing quote

    const std::string value =
        std::string(m_source.substr(m_start + 1, m_current - m_start - 2));
    add_token(token::Type::STRING, value);
  }

  void scan_number() {
    // Integer part
    while (is_digit(peek()))
      advance();

    // Fractional part
    if (peek() == '.' && is_digit(peek_next())) {
      advance();
      while (is_digit(peek()))
        advance();
    }

    const std::string_view raw_value =
        m_source.substr(m_start, m_current - m_start);
    try {
      const double value = std::stod(std::string{raw_value});
      add_token(token::Type::NUMBER, value);
    } catch (const std::invalid_argument &e) {
      throw ScannerError(m_line, m_column,
                  std::format("Invalid number: '{}'", raw_value));
    } catch (const std::out_of_range &e) {
      throw ScannerError(m_line, m_column,
                  std::format("Number out of range: '{}'", raw_value));
    }
  }

  void scan_identifier() {
    while (is_alphanumeric(peek()))
      advance();

    const std::string_view lexeme =
        m_source.substr(m_start, m_current - m_start);
    const auto keyword_it = keywords.find(lexeme);
    if (keyword_it != keywords.end()) {
      add_token(keyword_it->second);
    } else {
      add_token(token::Type::IDENTIFIER, lexeme);
    }
  }
};

} // namespace seam::scanner
