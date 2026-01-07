#include <any>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

import seam;
import seam.token;
import seam.scanner;
import seam.parser;
import seam.interpreter;
import seam.resolver;
import seam.ast;
import seam.error;
import seam.tests.fixtures;

using namespace seam;
using namespace seam::tests;

struct ControlFlow : public fixtures::Execution {};

TEST_F(ControlFlow, IfTrue) {
  std::string source = R"(
    if (true) print("yes");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "yes\n");
}

TEST_F(ControlFlow, IfFalse) {
  std::string source = R"(
    if (false) print("yes");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}

TEST_F(ControlFlow, IfElseTrue) {
  std::string source = R"(
    if (true) print("yes"); else print("no");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "yes\n");
}

TEST_F(ControlFlow, IfElseFalse) {
  std::string source = R"(
    if (false) print("yes"); else print("no");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "no\n");
}

TEST_F(ControlFlow, NestedIf) {
  std::string source = R"(
    if (true) {
      if (false) {
        print("inner-true");
      } else {
        print("inner-false");
      }
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "inner-false\n");
}

TEST_F(ControlFlow, IfWithBlock) {
  std::string source = R"(
    if (true) {
      print("a");
      print("b");
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\n");
}

TEST_F(ControlFlow, ElseIfChain) {
  std::string source = R"(
    let x = 2;
    if (x == 1) print("one");
    else if (x == 2) print("two");
    else if (x == 3) print("three");
    else print("other");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "two\n");
}

TEST_F(ControlFlow, IfConditionExpression) {
  std::string source = R"(
    if (5 > 3) print("big");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "big\n");
}

TEST_F(ControlFlow, WhileBasic) {
  std::string source = R"(
    let i = 1;
    while (i <= 3) {
      print(i);
      i = i + 1;
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n3\n");
}

TEST_F(ControlFlow, WhileNeverExecutes) {
  std::string source = R"(
    while (false) print("never");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}

TEST_F(ControlFlow, WhileWithBlock) {
  std::string source = R"(
    let i = 0;
    while (i < 2) {
      print("a");
      print("b");
      i = i + 1;
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\na\nb\n");
}

TEST_F(ControlFlow, WhileConditionUpdate) {
  std::string source = R"(
    let done = false;
    let count = 0;
    while (done == false) {
      count = count + 1;
      if (count >= 3) done = true;
    }
    print(count);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n");
}

TEST_F(ControlFlow, ForBasic) {
  std::string source = R"(
    let i = 0;
    for (i = 0; i < 3; i = i + 1) print(i);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n2\n");
}

TEST_F(ControlFlow, ForNoInitializer) {
  std::string source = R"(
    let i = 0;
    for (; i < 3; i = i + 1) print(i);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n2\n");
}

TEST_F(ControlFlow, ForExternalVariable) {
  std::string source = R"(
    let i = 0;
    for (i = 0; i < 3; i = i + 1) print(i);
    print(i);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n2\n3\n");
}

TEST_F(ControlFlow, ForWithBlock) {
  std::string source = R"(
    let i = 0;
    for (i = 0; i < 2; i = i + 1) {
      print("a");
      print(i);
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\n0\na\n1\n");
}

TEST_F(ControlFlow, NestedForLoops) {
  std::string source = R"(
    let i = 0;
    let j = 0;
    for (i = 0; i < 2; i = i + 1) {
      for (j = 0; j < 2; j = j + 1) {
        print(i * 10 + j);
      }
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n10\n11\n");
}

TEST_F(ControlFlow, ForScopeIsolation) {
  std::string source = R"(
    let i = "global";
    for (let i = 0; i < 2; i = i + 1) {
      print(i);
    }
    print(i);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\nglobal\n");
}

TEST_F(ControlFlow, ForBodyShadowing) {
  std::string source = R"(
    for (let i = 0; i < 1; i = i + 1) {
      let i = "shadow";
      print(i);
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "shadow\n");
}

TEST_F(ControlFlow, NestedForScopeShadowing) {
  std::string source = R"(
    for (let i = 0; i < 2; i = i + 1) {
      for (let i = 10; i < 11; i = i + 1) {
        print(i);
      }
      print(i);
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n0\n10\n1\n");
}

TEST_F(ControlFlow, ForWithConditionOnly) {
  std::string source = R"(
    let i = 0;
    for (; i < 3;) {
      print(i);
      i = i + 1;
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n2\n");
}
