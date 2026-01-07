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

class Lambdas : public fixtures::Execution {};

TEST_F(Lambdas, LambdaWithReturn) {
  std::string source = R"(
    var multiply = fun(a, b) {
      return a * b;
    };
    print multiply(6, 7);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Lambdas, LambdaAsReturn) {
  std::string source = R"(
    fun makeMultiplier(factor) {
      return fun(n) { return n * factor; };
    }
    var twice = makeMultiplier(2);
    print twice(5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Lambdas, NestedLambdas) {
  std::string source = R"(
    var f = fun(a) {
      return fun(b) {
        return fun(c) {
          return a + b + c;
        };
      };
    };
    print f(1)(2)(3);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "6\n");
}

TEST_F(Lambdas, LambdaCapturesThis) {
  std::string source = R"(
    class Greeter {
      init(name) {
        this.name = name;
      }
      getGreeter() {
        return fun() { print "Hi, " + this.name; };
      }
    }
    var g = Greeter("Seam");
    var f = g.getGreeter();
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Hi, Seam\n");
}

TEST_F(Lambdas, LambdaCaptureAndShadow) {
  std::string source = R"(
    var x = "outer";
    var f = nil;
    {
      var x = "inner";
      f = fun() { print x; };
    }
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "inner\n");
}

TEST_F(Lambdas, LambdaWithSideEffects) {
  std::string source = R"(
    var result = 0;
    var f = fun() { result = 42; };
    f();
    print result;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Lambdas, LambdaModifiesCapture) {
  std::string source = R"(
    var x = 10;
    var f = fun() { x = x + 5; };
    f();
    f();
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Lambdas, LambdaPrints) {
  std::string source = R"(
    var f = fun() { print "hello from lambda"; };
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello from lambda\n");
}

TEST_F(Lambdas, LambdaInVariable) {
  std::string source = R"(
    var greet = fun() { print "hi"; };
    greet();
    greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hi\nhi\n");
}

TEST_F(Lambdas, LambdaPassedToFunction) {
  std::string source = R"(
    var output = "";
    fun execute(f) {
      f();
    }
    execute(fun() { output = "executed"; });
    print output;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "executed\n");
}
