#include <gtest/gtest.h>

import seam.scanner;
import seam.token;
import seam.error;

using namespace seam;

TEST(Scanner, CanInstantiate) {
  scanner::Scanner scanner{"print(\"Hello, World!\")"};
  EXPECT_NO_THROW(scanner.scan_tokens());
}

TEST(Scanner, ScansSimpleTokens) {
  scanner::Scanner scanner{"(){},.+-;*"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 11); // 10 tokens + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::LEFT_PAREN);
  EXPECT_EQ(tokens[0].lexeme(), "(");
  EXPECT_EQ(tokens[1].type(), token::Type::RIGHT_PAREN);
  EXPECT_EQ(tokens[1].lexeme(), ")");
  EXPECT_EQ(tokens[2].type(), token::Type::LEFT_BRACE);
  EXPECT_EQ(tokens[2].lexeme(), "{");
  EXPECT_EQ(tokens[3].type(), token::Type::RIGHT_BRACE);
  EXPECT_EQ(tokens[3].lexeme(), "}");
  EXPECT_EQ(tokens[4].type(), token::Type::COMMA);
  EXPECT_EQ(tokens[4].lexeme(), ",");
  EXPECT_EQ(tokens[5].type(), token::Type::DOT);
  EXPECT_EQ(tokens[5].lexeme(), ".");
  EXPECT_EQ(tokens[6].type(), token::Type::PLUS);
  EXPECT_EQ(tokens[6].lexeme(), "+");
  EXPECT_EQ(tokens[7].type(), token::Type::MINUS);
  EXPECT_EQ(tokens[7].lexeme(), "-");
  EXPECT_EQ(tokens[8].type(), token::Type::SEMICOLON);
  EXPECT_EQ(tokens[8].lexeme(), ";");
  EXPECT_EQ(tokens[9].type(), token::Type::STAR);
  EXPECT_EQ(tokens[9].lexeme(), "*");
  EXPECT_EQ(tokens[10].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[10].lexeme(), "");
}

TEST(Scanner, ScansTwoCharacterTokens) {
  scanner::Scanner scanner{"!= == >= <="};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 5); // 4 tokens + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::BANG_EQUAL);
  EXPECT_EQ(tokens[0].lexeme(), "!=");
  EXPECT_EQ(tokens[1].type(), token::Type::EQUAL_EQUAL);
  EXPECT_EQ(tokens[1].lexeme(), "==");
  EXPECT_EQ(tokens[2].type(), token::Type::GREATER_EQUAL);
  EXPECT_EQ(tokens[2].lexeme(), ">=");
  EXPECT_EQ(tokens[3].type(), token::Type::LESS_EQUAL);
  EXPECT_EQ(tokens[3].lexeme(), "<=");
  EXPECT_EQ(tokens[4].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[4].lexeme(), "");
}

TEST(Scanner, ScansStringLiterals) {
  scanner::Scanner scanner{R"("hello world")"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 2); // string + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::STRING);
  EXPECT_EQ(tokens[0].lexeme(), R"("hello world")");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string>(tokens[0].literal().value()), "hello world");
  EXPECT_EQ(tokens[1].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[1].lexeme(), "");
}

TEST(Scanner, ScansNumberLiterals) {
  scanner::Scanner scanner{"42 3.14159"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // 2 numbers + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::NUMBER);
  EXPECT_EQ(tokens[0].lexeme(), "42");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<double>(tokens[0].literal().value()), 42.0);
  EXPECT_EQ(tokens[1].type(), token::Type::NUMBER);
  EXPECT_EQ(tokens[1].lexeme(), "3.14159");
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<double>(tokens[1].literal().value()), 3.14159);
  EXPECT_EQ(tokens[2].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[2].lexeme(), "");
}

TEST(Scanner, ScansIdentifiers) {
  scanner::Scanner scanner{"foo bar_baz _underscore"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 4); // 3 identifiers + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[0].lexeme(), "foo");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), "foo");
  EXPECT_EQ(tokens[1].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[1].lexeme(), "bar_baz");
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[1].literal().value()), "bar_baz");
  EXPECT_EQ(tokens[2].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[2].lexeme(), "_underscore");
  ASSERT_TRUE(tokens[2].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[2].literal().value()), "_underscore");
  EXPECT_EQ(tokens[3].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[3].lexeme(), "");
}

TEST(Scanner, ScansKeywords) {
  scanner::Scanner scanner{"if else while for static"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 6); // 5 keywords + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::IF);
  EXPECT_EQ(tokens[0].lexeme(), "if");
  EXPECT_EQ(tokens[1].type(), token::Type::ELSE);
  EXPECT_EQ(tokens[1].lexeme(), "else");
  EXPECT_EQ(tokens[2].type(), token::Type::WHILE);
  EXPECT_EQ(tokens[2].lexeme(), "while");
  EXPECT_EQ(tokens[3].type(), token::Type::FOR);
  EXPECT_EQ(tokens[3].lexeme(), "for");
  EXPECT_EQ(tokens[4].type(), token::Type::STATIC);
  EXPECT_EQ(tokens[4].lexeme(), "static");
  EXPECT_EQ(tokens[5].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[5].lexeme(), "");
}

TEST(Scanner, ScansLineComments) {
  scanner::Scanner scanner{"// self is a comment\nlet x"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 4); // comment + let + identifier + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "// self is a comment");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " self is a comment");
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
  EXPECT_EQ(tokens[1].lexeme(), "let");
  EXPECT_EQ(tokens[2].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[2].lexeme(), "x");
  ASSERT_TRUE(tokens[2].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[2].literal().value()), "x");
  EXPECT_EQ(tokens[3].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[3].lexeme(), "");
}

TEST(Scanner, ScansMultilineComments) {
  scanner::Scanner scanner{"/* comment */ let"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // comment + let + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "/* comment */");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " comment ");
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
  EXPECT_EQ(tokens[1].lexeme(), "let");
  EXPECT_EQ(tokens[2].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[2].lexeme(), "");
}

TEST(Scanner, ScansMultilineCommentsWithNewlines) {
  scanner::Scanner scanner{"/* self is a\nmultiline\ncomment */ let"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // comment + let + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "/* self is a\nmultiline\ncomment */");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " self is a\nmultiline\ncomment ");
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
  EXPECT_EQ(tokens[1].lexeme(), "let");
  EXPECT_EQ(tokens[2].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[2].lexeme(), "");
}

TEST(Scanner, ScansNestedMultilineComments) {
  scanner::Scanner scanner{"/* outer /* inner */ still outer */ let"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // comment + let + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "/* outer /* inner */ still outer */");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " outer /* inner */ still outer ");
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
  EXPECT_EQ(tokens[1].lexeme(), "let");
  EXPECT_EQ(tokens[2].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[2].lexeme(), "");
}

TEST(Scanner, ScansMultipleNestedMultilineComments) {
  scanner::Scanner scanner{"/* level 1 /* level 2 /* level 3 */ back to 2 */ back to 1 */ let"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // comment + let + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "/* level 1 /* level 2 /* level 3 */ back to 2 */ back to 1 */");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " level 1 /* level 2 /* level 3 */ back to 2 */ back to 1 ");
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
  EXPECT_EQ(tokens[1].lexeme(), "let");
  EXPECT_EQ(tokens[2].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[2].lexeme(), "");
}

TEST(Scanner, ScansMixedComments) {
  scanner::Scanner scanner{"// line comment\n/* block comment */ let // another line"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 5); // line comment + block comment + let + line comment + EOF
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "// line comment");
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " line comment");
  EXPECT_EQ(tokens[1].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[1].lexeme(), "/* block comment */");
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[1].literal().value()), " block comment ");
  EXPECT_EQ(tokens[2].type(), token::Type::LET);
  EXPECT_EQ(tokens[2].lexeme(), "let");
  EXPECT_EQ(tokens[3].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[3].lexeme(), "// another line");
  ASSERT_TRUE(tokens[3].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[3].literal().value()), " another line");
  EXPECT_EQ(tokens[4].type(), token::Type::END_OF_FILE);
  EXPECT_EQ(tokens[4].lexeme(), "");
}

TEST(Scanner, ThrowsOnUnterminatedString) {
  scanner::Scanner scanner{R"("unterminated)"};
  EXPECT_THROW(scanner.scan_tokens(), Error);
}

TEST(Scanner, ThrowsOnUnterminatedComment) {
  scanner::Scanner scanner{"/* unterminated"};
  EXPECT_THROW(scanner.scan_tokens(), Error);
}

TEST(Scanner, ThrowsOnUnexpectedCharacter) {
  scanner::Scanner scanner{"@"};
  EXPECT_THROW(scanner.scan_tokens(), Error);
}

TEST(Scanner, TracksLineAndColumnForSingleLineTokens) {
  scanner::Scanner scanner{"let x = 42"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 5); // let + x + = + 42 + EOF
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 5);
  EXPECT_EQ(tokens[2].line(), 1);
  EXPECT_EQ(tokens[2].column(), 7);
  EXPECT_EQ(tokens[3].line(), 1);
  EXPECT_EQ(tokens[3].column(), 9);
  EXPECT_EQ(tokens[4].line(), 1);
  EXPECT_EQ(tokens[4].column(), 10); // EOF is at the position after last char
}

TEST(Scanner, TracksLineAndColumnForMultiLineTokens) {
  scanner::Scanner scanner{"let x\nlet y\nlet z"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 7); // let + x + let + y + let + z + EOF
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  EXPECT_EQ(tokens[0].lexeme(), "let");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 5);
  EXPECT_EQ(tokens[1].lexeme(), "x");
  EXPECT_EQ(tokens[2].line(), 2);
  EXPECT_EQ(tokens[2].column(), 1);
  EXPECT_EQ(tokens[2].lexeme(), "let");
  EXPECT_EQ(tokens[3].line(), 2);
  EXPECT_EQ(tokens[3].column(), 5);
  EXPECT_EQ(tokens[3].lexeme(), "y");
  EXPECT_EQ(tokens[4].line(), 3);
  EXPECT_EQ(tokens[4].column(), 1);
  EXPECT_EQ(tokens[4].lexeme(), "let");
  EXPECT_EQ(tokens[5].line(), 3);
  EXPECT_EQ(tokens[5].column(), 5);
  EXPECT_EQ(tokens[5].lexeme(), "z");
}

TEST(Scanner, TracksLineAndColumnForMultiLineStrings) {
  scanner::Scanner scanner{R"("hello\nworld")"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 2); // string + EOF
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  EXPECT_EQ(tokens[0].type(), token::Type::STRING);
}

TEST(Scanner, TracksLineAndColumnForMultiLineComments) {
  scanner::Scanner scanner{"/* line 1\nline 2\nline 3 */ let"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3); // comment + let + EOF
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[1].line(), 3);
  EXPECT_EQ(tokens[1].column(), 11); // let starts after "line 3 */ "
  EXPECT_EQ(tokens[1].type(), token::Type::LET);
}

TEST(Scanner, TracksLineAndColumnWithMixedWhitespace) {
  scanner::Scanner scanner{"  let\t\tx  \n\t y"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 4); // let + x + y + EOF
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 3);
  EXPECT_EQ(tokens[0].lexeme(), "let");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 8); // After "let" and two tabs
  EXPECT_EQ(tokens[1].lexeme(), "x");
  EXPECT_EQ(tokens[2].line(), 2);
  EXPECT_EQ(tokens[2].column(), 3); // After one tab on line 2
  EXPECT_EQ(tokens[2].lexeme(), "y");
}

TEST(Scanner, VerifiesTokenDataForOperators) {
  scanner::Scanner scanner{"!= == >= <="};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 5);

  EXPECT_EQ(tokens[0].type(), token::Type::BANG_EQUAL);
  EXPECT_EQ(tokens[0].lexeme(), "!=");
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  EXPECT_FALSE(tokens[0].literal().has_value());

  EXPECT_EQ(tokens[1].type(), token::Type::EQUAL_EQUAL);
  EXPECT_EQ(tokens[1].lexeme(), "==");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 4);
  EXPECT_FALSE(tokens[1].literal().has_value());

  EXPECT_EQ(tokens[2].type(), token::Type::GREATER_EQUAL);
  EXPECT_EQ(tokens[2].lexeme(), ">=");
  EXPECT_EQ(tokens[2].line(), 1);
  EXPECT_EQ(tokens[2].column(), 7);
  EXPECT_FALSE(tokens[2].literal().has_value());

  EXPECT_EQ(tokens[3].type(), token::Type::LESS_EQUAL);
  EXPECT_EQ(tokens[3].lexeme(), "<=");
  EXPECT_EQ(tokens[3].line(), 1);
  EXPECT_EQ(tokens[3].column(), 10);
  EXPECT_FALSE(tokens[3].literal().has_value());
}

TEST(Scanner, VerifiesTokenDataForStringLiterals) {
  scanner::Scanner scanner{R"("test" "hello world")"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3);

  EXPECT_EQ(tokens[0].type(), token::Type::STRING);
  EXPECT_EQ(tokens[0].lexeme(), R"("test")");
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string>(tokens[0].literal().value()), "test");

  EXPECT_EQ(tokens[1].type(), token::Type::STRING);
  EXPECT_EQ(tokens[1].lexeme(), R"("hello world")");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 8); // After "test" (6 chars) and space
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string>(tokens[1].literal().value()), "hello world");
}

TEST(Scanner, VerifiesTokenDataForNumberLiterals) {
  scanner::Scanner scanner{"123 456.789"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 3);

  EXPECT_EQ(tokens[0].type(), token::Type::NUMBER);
  EXPECT_EQ(tokens[0].lexeme(), "123");
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<double>(tokens[0].literal().value()), 123.0);

  EXPECT_EQ(tokens[1].type(), token::Type::NUMBER);
  EXPECT_EQ(tokens[1].lexeme(), "456.789");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 5);
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<double>(tokens[1].literal().value()), 456.789);
}

TEST(Scanner, VerifiesTokenDataForIdentifiersAndKeywords) {
  scanner::Scanner scanner{"myLet if else myFunctionc"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 5);

  EXPECT_EQ(tokens[0].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[0].lexeme(), "myLet");
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), "myLet");

  EXPECT_EQ(tokens[1].type(), token::Type::IF);
  EXPECT_EQ(tokens[1].lexeme(), "if");
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 7);
  EXPECT_FALSE(tokens[1].literal().has_value());

  EXPECT_EQ(tokens[2].type(), token::Type::ELSE);
  EXPECT_EQ(tokens[2].lexeme(), "else");
  EXPECT_EQ(tokens[2].line(), 1);
  EXPECT_EQ(tokens[2].column(), 10);
  EXPECT_FALSE(tokens[2].literal().has_value());

  EXPECT_EQ(tokens[3].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[3].lexeme(), "myFunctionc");
  EXPECT_EQ(tokens[3].line(), 1);
  EXPECT_EQ(tokens[3].column(), 15);
  ASSERT_TRUE(tokens[3].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[3].literal().value()), "myFunctionc");
}

TEST(Scanner, VerifiesTokenDataForComments) {
  scanner::Scanner scanner{"// comment\n/* block */\nlet"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 4);

  EXPECT_EQ(tokens[0].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[0].lexeme(), "// comment");
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);
  ASSERT_TRUE(tokens[0].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[0].literal().value()), " comment");

  EXPECT_EQ(tokens[1].type(), token::Type::COMMENT);
  EXPECT_EQ(tokens[1].lexeme(), "/* block */");
  EXPECT_EQ(tokens[1].line(), 2);
  EXPECT_EQ(tokens[1].column(), 1);
  ASSERT_TRUE(tokens[1].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string_view>(tokens[1].literal().value()), " block ");

  EXPECT_EQ(tokens[2].type(), token::Type::LET);
  EXPECT_EQ(tokens[2].lexeme(), "let");
  EXPECT_EQ(tokens[2].line(), 3);
  EXPECT_EQ(tokens[2].column(), 1);
}

TEST(Scanner, VerifiesComplexTokenSequenceData) {
  scanner::Scanner scanner{"if (x >= 10) {\n  print(\"big\");\n}"};
  auto tokens = scanner.scan_tokens();

  ASSERT_EQ(tokens.size(), 14); // 13 tokens + EOF

  // if
  EXPECT_EQ(tokens[0].type(), token::Type::IF);
  EXPECT_EQ(tokens[0].line(), 1);
  EXPECT_EQ(tokens[0].column(), 1);

  // (
  EXPECT_EQ(tokens[1].type(), token::Type::LEFT_PAREN);
  EXPECT_EQ(tokens[1].line(), 1);
  EXPECT_EQ(tokens[1].column(), 4);

  // x
  EXPECT_EQ(tokens[2].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[2].lexeme(), "x");
  EXPECT_EQ(tokens[2].line(), 1);
  EXPECT_EQ(tokens[2].column(), 5);

  // >=
  EXPECT_EQ(tokens[3].type(), token::Type::GREATER_EQUAL);
  EXPECT_EQ(tokens[3].line(), 1);
  EXPECT_EQ(tokens[3].column(), 7);

  // 10
  EXPECT_EQ(tokens[4].type(), token::Type::NUMBER);
  EXPECT_EQ(tokens[4].lexeme(), "10");
  EXPECT_EQ(tokens[4].line(), 1);
  EXPECT_EQ(tokens[4].column(), 10);
  ASSERT_TRUE(tokens[4].literal().has_value());
  EXPECT_EQ(std::any_cast<double>(tokens[4].literal().value()), 10.0);

  // )
  EXPECT_EQ(tokens[5].type(), token::Type::RIGHT_PAREN);
  EXPECT_EQ(tokens[5].line(), 1);
  EXPECT_EQ(tokens[5].column(), 12);

  // {
  EXPECT_EQ(tokens[6].type(), token::Type::LEFT_BRACE);
  EXPECT_EQ(tokens[6].line(), 1);
  EXPECT_EQ(tokens[6].column(), 14);

  // print
  EXPECT_EQ(tokens[7].type(), token::Type::IDENTIFIER);
  EXPECT_EQ(tokens[7].line(), 2);
  EXPECT_EQ(tokens[7].column(), 3);

  // (
  EXPECT_EQ(tokens[8].type(), token::Type::LEFT_PAREN);
  EXPECT_EQ(tokens[8].line(), 2);
  EXPECT_EQ(tokens[8].column(), 8);

  // "big"
  EXPECT_EQ(tokens[9].type(), token::Type::STRING);
  EXPECT_EQ(tokens[9].lexeme(), R"("big")");
  EXPECT_EQ(tokens[9].line(), 2);
  EXPECT_EQ(tokens[9].column(), 9);
  ASSERT_TRUE(tokens[9].literal().has_value());
  EXPECT_EQ(std::any_cast<std::string>(tokens[9].literal().value()), "big");

  // )
  EXPECT_EQ(tokens[10].type(), token::Type::RIGHT_PAREN);
  EXPECT_EQ(tokens[10].line(), 2);
  EXPECT_EQ(tokens[10].column(), 14);

  // ;
  EXPECT_EQ(tokens[11].type(), token::Type::SEMICOLON);
  EXPECT_EQ(tokens[11].line(), 2);
  EXPECT_EQ(tokens[11].column(), 15);

  // }
  EXPECT_EQ(tokens[12].type(), token::Type::RIGHT_BRACE);
  EXPECT_EQ(tokens[12].line(), 3);
  EXPECT_EQ(tokens[12].column(), 1);
}
