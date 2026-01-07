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

class EdgeCases : public fixtures::Execution {};

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
    var a = 1;
    {
      var b = 2;
      print a + b;
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n");
}

TEST_F(EdgeCases, LongChainedCalls) {
  std::string source = R"(
    class Builder {
      init() {
        this.value = 0;
      }
      add(n) {
        this.value = this.value + n;
        return this;
      }
      get() {
        return this.value;
      }
    }
    print Builder().add(1).add(2).add(3).add(4).get();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(EdgeCases, FunctionAsProperty) {
  std::string source = R"(
    class Container {}
    fun greet() {
      return "hello";
    }
    var c = Container();
    c.fn = greet;
    print c.fn();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(EdgeCases, SelfReferenceInUpdate) {
  std::string source = R"(
    var x = 5;
    x = x + x;
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(EdgeCases, ZeroArityFunction) {
  std::string source = R"(
    fun noArgs() {
      return 42;
    }
    print noArgs();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(EdgeCases, ComplexTernaryNesting) {
  std::string source = R"(
    var a = true;
    var b = false;
    var c = true;
    print a ? (b ? 1 : (c ? 2 : 3)) : 4;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n");
}

TEST_F(EdgeCases, MixedArithmeticAndComparison) {
  std::string source = R"(
    print 1 + 2 * 3 > 5 and 10 / 2 == 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(EdgeCases, NestedFunctionCalls) {
  std::string source = R"(
    fun add(a, b) { return a + b; }
    fun mul(a, b) { return a * b; }
    print add(mul(2, 3), mul(4, 5));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "26\n");
}

TEST_F(EdgeCases, ClassWithManyMethods) {
  std::string source = R"(
    class Calculator {
      init() { this.result = 0; }
      add(n) { this.result = this.result + n; return this; }
      sub(n) { this.result = this.result - n; return this; }
      mul(n) { this.result = this.result * n; return this; }
      div(n) { this.result = this.result / n; return this; }
      get() { return this.result; }
    }
    var c = Calculator();
    print c.add(10).mul(2).sub(5).div(3).get();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "5\n");
}

TEST_F(EdgeCases, InheritedMethodCallsOverriddenMethod) {
  std::string source = R"(
    class Base {
      method() { this.printMe(); }
      printMe() { print "base"; }
    }
    class Derived + Base {
      printMe() { print "derived"; }
    }
    Derived().method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "derived\n");
}

TEST_F(EdgeCases, DeeplyNestedExpressions) {
  std::string source = R"(
    print 1 + (2 + (3 + (4 + (5 + (6 + 7)))));
    print (true ? (false ? "a" : (true ? "b" : "c")) : "d");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "28\nb\n");
}

TEST_F(EdgeCases, ReassignFunctionVariable) {
  std::string source = R"(
    fun first() { return 1; }
    fun second() { return 2; }
    var f = first;
    print f();
    f = second;
    print f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n");
}

TEST_F(EdgeCases, WhitespaceOnly) {
  std::string source = "   \n\t\n   ";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "");
}
