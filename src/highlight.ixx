module;

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <replxx.hxx>

export module seam.highlight;

import seam.common;
import seam.token;
import seam.scanner;
import seam.error;

export namespace seam::highlight {

enum class TokenCategory : u8 {
  KEYWORD,
  OPERATOR,
  IDENTIFIER,
  NUMBER,
  STRING,
  BOOLEAN,
  NIL,
  COMMENT,
  DELIMITER,
  PUNCTUATION,
  SELF,
  PARENT,
};

enum class TokenModifier : u8 {
  NONE = 0,
  DECLARATION = 1 << 0,
  STATIC = 1 << 1,
  BUILTIN = 1 << 2,
};

struct HighlightSpan {
  usize start_offset;
  usize length;
  usize line;
  usize column;
  TokenCategory category;
  TokenModifier modifiers = TokenModifier::NONE;
};

struct HighlightResult {
  std::vector<HighlightSpan> spans;
  bool has_errors = false;
  usize error_offset = 0;
};

inline usize calculate_offset(std::string_view source, usize line,
                              usize column) {
  usize offset = 0;
  usize current_line = 1;

  while (offset < source.length() && current_line < line) {
    if (source[offset] == '\n') {
      current_line++;
    }
    offset++;
  }

  offset += column - 1;

  return offset < source.length() ? offset : source.length();
}

class Highlighter {
public:
  HighlightResult highlight(std::string_view source) const {
    HighlightResult result;

    try {
      scanner::Scanner scanner{source};
      auto tokens = scanner.scan_tokens();

      for (const auto &tok : tokens) {
        if (tok.type() == token::Type::END_OF_FILE)
          break;

        HighlightSpan span;
        span.line = tok.line();
        span.column = tok.column();
        span.length = tok.lexeme().length();
        span.category = categorize(tok.type());
        span.modifiers = TokenModifier::NONE;
        span.start_offset = calculate_offset(source, tok.line(), tok.column());

        result.spans.emplace_back(std::move(span));
      }
    } catch (const Error &e) {
      result.has_errors = true;
      result.error_offset =
          calculate_offset(source, static_cast<usize>(e.location().line),
                           static_cast<usize>(e.location().column));
    }

    return result;
  }

private:
  TokenCategory categorize(token::Type type) const {
    switch (type) {
    // Keywords
    case token::Type::AND:
    case token::Type::OR:
    case token::Type::IF:
    case token::Type::ELSE:
    case token::Type::WHILE:
    case token::Type::FOR:
    case token::Type::FN:
    case token::Type::LET:
    case token::Type::STRUCT:
    case token::Type::STATIC:
    case token::Type::RETURN:
      return TokenCategory::KEYWORD;

    case token::Type::TRUE:
    case token::Type::FALSE:
      return TokenCategory::BOOLEAN;

    case token::Type::NIL:
      return TokenCategory::NIL;

    case token::Type::SELF:
      return TokenCategory::SELF;

    case token::Type::PARENT:
      return TokenCategory::PARENT;

    // Operators
    case token::Type::PLUS:
    case token::Type::MINUS:
    case token::Type::STAR:
    case token::Type::SLASH:
    case token::Type::BANG:
    case token::Type::BANG_EQUAL:
    case token::Type::EQUAL:
    case token::Type::EQUAL_EQUAL:
    case token::Type::GREATER:
    case token::Type::GREATER_EQUAL:
    case token::Type::LESS:
    case token::Type::LESS_EQUAL:
      return TokenCategory::OPERATOR;

    // Literals
    case token::Type::NUMBER:
      return TokenCategory::NUMBER;
    case token::Type::STRING:
      return TokenCategory::STRING;
    case token::Type::COMMENT:
      return TokenCategory::COMMENT;
    case token::Type::IDENTIFIER:
      return TokenCategory::IDENTIFIER;

    // Delimiters
    case token::Type::LEFT_PAREN:
    case token::Type::RIGHT_PAREN:
    case token::Type::LEFT_BRACE:
    case token::Type::RIGHT_BRACE:
      return TokenCategory::DELIMITER;

    // Punctuation
    case token::Type::COMMA:
    case token::Type::DOT:
    case token::Type::SEMICOLON:
    case token::Type::COLON:
    case token::Type::QUESTION_MARK:
      return TokenCategory::PUNCTUATION;

    default:
      return TokenCategory::IDENTIFIER;
    }
  }
};

class ReplxxConverter {
public:
  void apply(const HighlightResult &result, std::string_view source,
             replxx::Replxx::colors_t &colors) const {
    std::vector<usize> byte_to_cp;
    byte_to_cp.reserve(source.length());

    usize cp_index = 0;
    for (usize i = 0; i < source.length();) {
      byte_to_cp.emplace_back(cp_index);

      unsigned char c = static_cast<unsigned char>(source[i]);
      if ((c & 0x80) == 0)
        i += 1;
      else if ((c & 0xE0) == 0xC0)
        i += 2;
      else if ((c & 0xF0) == 0xE0)
        i += 3;
      else
        i += 4;
      cp_index++;
    }
    byte_to_cp.emplace_back(cp_index);

    for (const auto &span : result.spans) {
      auto color = category_to_color(span.category);

      usize start_byte = span.start_offset;
      usize end_byte = span.start_offset + span.length;

      if (start_byte >= byte_to_cp.size())
        continue;

      usize start_cp = byte_to_cp[start_byte];
      usize end_cp =
          (end_byte < byte_to_cp.size()) ? byte_to_cp[end_byte] : cp_index;

      for (usize i = start_cp; i < end_cp && i < colors.size(); ++i) {
        colors[i] = color;
      }
    }
  }

private:
  replxx::Replxx::Color category_to_color(TokenCategory cat) const {
    using Color = replxx::Replxx::Color;

    switch (cat) {
    case TokenCategory::KEYWORD:
    case TokenCategory::SELF:
    case TokenCategory::PARENT:
      return replxx::color::bold(Color::BRIGHTMAGENTA);

    case TokenCategory::OPERATOR:
      return replxx::color::bold(Color::BRIGHTCYAN);

    case TokenCategory::IDENTIFIER:
      return Color::WHITE;

    case TokenCategory::NUMBER:
      return Color::BRIGHTGREEN;

    case TokenCategory::STRING:
      return Color::GREEN;

    case TokenCategory::BOOLEAN:
      return Color::BRIGHTBLUE;

    case TokenCategory::NIL:
    case TokenCategory::COMMENT:
      return Color::GRAY;

    case TokenCategory::DELIMITER:
    case TokenCategory::PUNCTUATION:
    default:
      return Color::DEFAULT;
    }
  }
};

class LspConverter {
public:
  static constexpr u32 TYPE_KEYWORD = 0;
  static constexpr u32 TYPE_OPERATOR = 1;
  static constexpr u32 TYPE_VARIABLE = 2;
  static constexpr u32 TYPE_NUMBER = 3;
  static constexpr u32 TYPE_STRING = 4;
  static constexpr u32 TYPE_COMMENT = 5;

  static constexpr u32 MOD_DECLARATION = 1 << 0;
  static constexpr u32 MOD_STATIC = 1 << 1;

  std::vector<u32> convert(const HighlightResult &result) const {
    std::vector<u32> data;
    data.reserve(result.spans.size() * 5);

    usize prev_line = 0;
    usize prev_start = 0;

    for (const auto &span : result.spans) {
      if (span.category == TokenCategory::DELIMITER ||
          span.category == TokenCategory::PUNCTUATION) {
        continue;
      }

      // LSP uses 0-indexed lines/columns
      usize line = span.line - 1;
      usize start = span.column - 1;

      u32 delta_line = static_cast<u32>(line - prev_line);
      u32 delta_start = (delta_line == 0) ? static_cast<u32>(start - prev_start)
                                          : static_cast<u32>(start);

      data.emplace_back(delta_line);
      data.emplace_back(delta_start);
      data.emplace_back(static_cast<u32>(span.length));
      data.emplace_back(category_to_type(span.category));
      data.emplace_back(modifiers_to_bitmask(span.modifiers));

      prev_line = line;
      prev_start = start;
    }

    return data;
  }

  static std::vector<std::string> token_types_legend() {
    return {"keyword", "operator", "variable", "number", "string", "comment"};
  }

  static std::vector<std::string> token_modifiers_legend() {
    return {"declaration", "static"};
  }

private:
  u32 category_to_type(TokenCategory cat) const {
    switch (cat) {
    case TokenCategory::KEYWORD:
    case TokenCategory::BOOLEAN:
    case TokenCategory::NIL:
    case TokenCategory::SELF:
    case TokenCategory::PARENT:
      return TYPE_KEYWORD;

    case TokenCategory::OPERATOR:
      return TYPE_OPERATOR;

    case TokenCategory::IDENTIFIER:
      return TYPE_VARIABLE;

    case TokenCategory::NUMBER:
      return TYPE_NUMBER;

    case TokenCategory::STRING:
      return TYPE_STRING;

    case TokenCategory::COMMENT:
      return TYPE_COMMENT;

    default:
      return TYPE_VARIABLE;
    }
  }

  u32 modifiers_to_bitmask(TokenModifier mods) const {
    u32 result = 0;
    if (static_cast<u8>(mods) & static_cast<u8>(TokenModifier::DECLARATION)) {
      result |= MOD_DECLARATION;
    }
    if (static_cast<u8>(mods) & static_cast<u8>(TokenModifier::STATIC)) {
      result |= MOD_STATIC;
    }
    return result;
  }
};

} // namespace seam::highlight
