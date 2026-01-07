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

struct Expressions : public fixtures::Execution {};

// Arithmetic Operators

TEST_F(Expressions, Addition) {
  std::string source = R"(
    print 5 + 3;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "8\n");
}

TEST_F(Expressions, Subtraction) {
  std::string source = R"(
    print 10 - 4;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "6\n");
}

TEST_F(Expressions, Multiplication) {
  std::string source = R"(
    print 6 * 7;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Expressions, Division) {
  std::string source = R"(
    print 15 / 3;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "5\n");
}

TEST_F(Expressions, FloatArithmetic) {
  std::string source = R"(
    print 3.5 * 2;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "7\n");
}

TEST_F(Expressions, Precedence) {
  std::string source = R"(
    print 2 + 3 * 4;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "14\n");
}

TEST_F(Expressions, GroupingOverrides) {
  std::string source = R"(
    print (2 + 3) * 4;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Expressions, ComplexExpression) {
  std::string source = R"(
    print 10 - 2 * 3 + 4 / 2;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "6\n");
}

// Comparison Operators

TEST_F(Expressions, GreaterThan) {
  std::string source = R"(
    print 5 > 3;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, GreaterThanFalse) {
  std::string source = R"(
    print 3 > 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "false\n");
}

TEST_F(Expressions, GreaterEqual) {
  std::string source = R"(
    print 5 >= 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, LessThan) {
  std::string source = R"(
    print 3 < 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, LessEqual) {
  std::string source = R"(
    print 5 <= 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, EqualNumbers) {
  std::string source = R"(
    print 5 == 5;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, EqualStrings) {
  std::string source = R"(
    print "a" == "a";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, EqualBooleans) {
  std::string source = R"(
    print true == true;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, NotEqual) {
  std::string source = R"(
    print 5 != 3;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, NilEquality) {
  std::string source = R"(
    print nil == nil;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, DifferentTypesNotEqual) {
  std::string source = R"(
    print 5 == "5";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "false\n");
}

// String Concatenation

TEST_F(Expressions, StringConcat) {
  std::string source = R"(
    print "Hello, " + "World!";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Hello, World!\n");
}

// Unary Operators

TEST_F(Expressions, Negation) {
  std::string source = R"(
    let x = 0 - 5;
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "-5\n");
}

TEST_F(Expressions, DoubleNegation) {
  std::string source = R"(
    let x = 0 - 0 - 5;
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "-5\n");
}

TEST_F(Expressions, BangInCondition) {
  std::string source = R"(
    if (true == false) {
      print "yes";
    } else {
      print "no";
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "no\n");
}

TEST_F(Expressions, NotEqualComparison) {
  std::string source = R"(
    print true != false;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, FalsyNil) {
  std::string source = R"(
    if (nil) {
      print "truthy";
    } else {
      print "falsy";
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "falsy\n");
}

TEST_F(Expressions, TruthyTrue) {
  std::string source = R"(
    if (true) {
      print "truthy";
    } else {
      print "falsy";
    }
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "truthy\n");
}

// Ternary Operator

TEST_F(Expressions, TernaryTrue) {
  std::string source = R"(
    print true ? "yes" : "no";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "yes\n");
}

TEST_F(Expressions, TernaryFalse) {
  std::string source = R"(
    print false ? "yes" : "no";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "no\n");
}

TEST_F(Expressions, NestedTernary) {
  std::string source = R"(
    print true ? (false ? 1 : 2) : 3;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n");
}

TEST_F(Expressions, TernaryWithExpression) {
  std::string source = R"(
    print 5 > 3 ? "big" : "small";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "big\n");
}

// Logical Operators

TEST_F(Expressions, AndTrueTrue) {
  std::string source = R"(
    print true and true;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, AndTrueFalse) {
  std::string source = R"(
    print true and false;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "false\n");
}

TEST_F(Expressions, AndShortCircuit) {
  std::string source = R"(
    let x = 0;
    false and (x = 1);
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n");
}

TEST_F(Expressions, OrFalseTrue) {
  std::string source = R"(
    print false or true;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, OrTrueFalse) {
  std::string source = R"(
    print true or false;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(Expressions, OrShortCircuit) {
  std::string source = R"(
    let x = 0;
    true or (x = 1);
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n");
}

TEST_F(Expressions, LogicalReturnsValue) {
  std::string source = R"(
    print nil or "default";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "default\n");
}

// Error Cases

TEST_F(Expressions, SubtractStringsError) {
  std::string source = R"(
    print "a" - "b";
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Expressions, NegateStringError) {
  std::string source = R"(
    let s = "not a number";
    print -s;
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Expressions, UnaryMinus) {
  std::string source = R"(
    print -5;
    print --5;
    print ---5;
    print -(5 + 5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "-5\n5\n-5\n-10\n");
}

TEST_F(Expressions, UnaryBang) {
  std::string source = R"(
    print !true;
    print !false;
    print !!true;
    print !!!true;
    print !(5 == 5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "false\ntrue\ntrue\nfalse\nfalse\n");
}

TEST_F(Expressions, LogicalShortCircuitSideEffects) {
  std::string source = R"(
    let x = 0;
    fn increment() {
      x = x + 1;
      return true;
    }

    false and increment();
    print x;
    true or increment();
    print x;
    true and increment();
    print x;
    false or increment();
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n0\n1\n2\n");
}

TEST_F(Expressions, TernaryRightAssociativity) {
  std::string source = R"(
    print true ? "a" : false ? "b" : "c";
    print false ? "a" : true ? "b" : "c";
    print false ? "a" : false ? "b" : "c";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\nc\n");
}
