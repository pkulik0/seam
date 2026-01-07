#include <gtest/gtest.h>

import seam.parser;
import seam.token;

using namespace seam;

TEST(Parser, CanInstantiate) {
  std::vector<token::Token> tokens;
  tokens.emplace_back(token::Type::TRUE, "true", std::nullopt, 1, 1);
  tokens.emplace_back(token::Type::END_OF_FILE, "", std::nullopt, 1, 5);
  parser::Parser parser{tokens};
  EXPECT_NO_THROW(parser.parse());
}

TEST(Parser, ParsesTernaryExpression) {
  std::vector<token::Token> tokens;
  tokens.emplace_back(token::Type::TRUE, "true", std::nullopt, 1, 1);
  tokens.emplace_back(token::Type::QUESTION_MARK, "?", std::nullopt, 1, 5);
  tokens.emplace_back(token::Type::TRUE, "true", std::nullopt, 1, 7);
  tokens.emplace_back(token::Type::COLON, ":", std::nullopt, 1, 10);
  tokens.emplace_back(token::Type::FALSE, "false", std::nullopt, 1, 12);
  tokens.emplace_back(token::Type::END_OF_FILE, "", std::nullopt, 1, 16);
  parser::Parser parser{tokens};
  EXPECT_NO_THROW(parser.parse());
}

TEST(Parser, ParsesTernaryExpressionWithFalseCondition) {
  std::vector<token::Token> tokens;
  tokens.emplace_back(token::Type::FALSE, "false", std::nullopt, 1, 1);
  tokens.emplace_back(token::Type::QUESTION_MARK, "?", std::nullopt, 1, 5);
  tokens.emplace_back(token::Type::TRUE, "true", std::nullopt, 1, 7);
  tokens.emplace_back(token::Type::COLON, ":", std::nullopt, 1, 10);
  tokens.emplace_back(token::Type::FALSE, "false", std::nullopt, 1, 12);
  tokens.emplace_back(token::Type::END_OF_FILE, "", std::nullopt, 1, 16);
  parser::Parser parser{tokens};
  EXPECT_NO_THROW(parser.parse());
}
