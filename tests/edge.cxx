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

struct EdgeCases : public fixtures::Execution {};

TEST_F(EdgeCases, EmptyProgram) {
  std::string source = "";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}

TEST_F(EdgeCases, OnlyLineComments) {
  std::string source = R"(
    // This is a comment
    // Another comment
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}

TEST_F(EdgeCases, OnlyBlockComments) {
  std::string source = R"(
    /* This is a block comment */
    /* Another
       multiline
       comment */
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}

TEST_F(EdgeCases, SingleNestedScope) {
  std::string source = R"(
    let a = 1;
    {
      let b = 2;
      print(a + b);
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n");
}

TEST_F(EdgeCases, LongChainedCalls) {
  std::string source = R"(
    struct Builder {
      init() {
        self.value = 0;
      }
      add(n) {
        self.value = self.value + n;
        return self;
      }
      get() {
        return self.value;
      }
    }
    print(Builder().add(1).add(2).add(3).add(4).get());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(EdgeCases, FunctionAsProperty) {
  std::string source = R"(
    struct Container {}
    fn greet() {
      return "hello";
    }
    let c = Container();
    c.func = greet;
    print(c.func());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(EdgeCases, SelfReferenceInUpdate) {
  std::string source = R"(
    let x = 5;
    x = x + x;
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(EdgeCases, ZeroArityFunction) {
  std::string source = R"(
    fn noArgs() {
      return 42;
    }
    print(noArgs());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(EdgeCases, ComplexTernaryNesting) {
  std::string source = R"(
    let a = true;
    let b = false;
    let c = true;
    print(a ? (b ? 1 : (c ? 2 : 3)) : 4);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n");
}

TEST_F(EdgeCases, MixedArithmeticAndComparison) {
  std::string source = R"(
    print(1 + 2 * 3 > 5 and 10 / 2 == 5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(EdgeCases, NestedFunctionCalls) {
  std::string source = R"(
    fn add(a, b) { return a + b; }
    fn mul(a, b) { return a * b; }
    print(add(mul(2, 3), mul(4, 5)));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "26\n");
}

TEST_F(EdgeCases, StructWithManyMethods) {
  std::string source = R"(
    struct Calculator {
      init() { self.result = 0; }
      add(n) { self.result = self.result + n; return self; }
      sub(n) { self.result = self.result - n; return self; }
      mul(n) { self.result = self.result * n; return self; }
      div(n) { self.result = self.result / n; return self; }
      get() { return self.result; }
    }
    let c = Calculator();
    print(c.add(10).mul(2).sub(5).div(3).get());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "5\n");
}

TEST_F(EdgeCases, InheritedMethodCallsOverriddenMethod) {
  std::string source = R"(
    struct Base {
      method() { self.printMe(); }
      printMe() { print("base"); }
    }
    struct Derived + Base {
      printMe() { print("derived"); }
    }
    Derived().method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "derived\n");
}

TEST_F(EdgeCases, DeeplyNestedExpressions) {
  std::string source = R"(
    print(1 + (2 + (3 + (4 + (5 + (6 + 7))))));
    print((true ? (false ? "a" : (true ? "b" : "c")) : "d"));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "28\nb\n");
}

TEST_F(EdgeCases, ReassignFunctionVariable) {
  std::string source = R"(
    fn first() { return 1; }
    fn second() { return 2; }
    let f = first;
    print(f());
    f = second;
    print(f());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n");
}

TEST_F(EdgeCases, WhitespaceOnly) {
  std::string source = "   \n\t\n   ";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}
