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

struct Variables : public fixtures::Execution {};

TEST_F(Variables, DeclarationAndAccess) {
  std::string source = R"(
    let x = 10;
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Variables, InitializerExpression) {
  std::string source = R"(
    let x = 5 + 5;
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Variables, Reassignment) {
  std::string source = R"(
    let x = 1;
    x = 2;
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n");
}

TEST_F(Variables, StringVariable) {
  std::string source = R"(
    let s = "hello";
    print(s);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(Variables, BooleanVariable) {
  std::string source = R"(
    let b = true;
    print(b);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Variables, NilVariable) {
  std::string source = R"(
    let n = nil;
    print(n);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "nil\n");
}

TEST_F(Variables, LocalScopeShadowing) {
  std::string source = R"(
    let a = "outer";
    {
      let a = "inner";
      print(a);
    }
    print(a);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "inner\nouter\n");
}

TEST_F(Variables, NestedScopes) {
  std::string source = R"(
    let a = "global";
    {
      let a = "outer";
      {
        let a = "inner";
        print(a);
      }
      print(a);
    }
    print(a);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "inner\nouter\nglobal\n");
}

TEST_F(Variables, DeeplyNestedShadowing) {
  std::string source = R"(
    let x = 1;
    {
      let x = 2;
      {
        let x = 3;
        {
          let x = 4;
          print(x);
        }
        print(x);
      }
      print(x);
    }
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "4\n3\n2\n1\n");
}

TEST_F(Variables, ShadowingInDifferentBranches) {
  std::string source = R"(
    let x = "global";
    if (true) {
      let x = "then";
      print(x);
    }
    if (true) {
      let x = "else";
      print(x);
    }
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "then\nelse\nglobal\n");
}

TEST_F(Variables, BlockAccessesOuter) {
  std::string source = R"(
    let x = 10;
    {
      print(x);
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Variables, BlockModifiesOuter) {
  std::string source = R"(
    let x = 10;
    {
      x = 20;
    }
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Variables, UndefinedVariableError) {
  std::string source = R"(
    print(x);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Variables, AssignUndefinedError) {
  std::string source = R"(
    x = 10;
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Variables, NilInitializedVariable) {
  std::string source = R"(
    let x = nil;
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "nil\n");
}
