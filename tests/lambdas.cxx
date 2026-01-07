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

struct Lambdas : public fixtures::Execution {};

TEST_F(Lambdas, LambdaWithReturn) {
  std::string source = R"(
    let multiply = fn(a, b) {
      return a * b;
    };
    print multiply(6, 7);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Lambdas, LambdaAsReturn) {
  std::string source = R"(
    fn makeMultiplier(factor) {
      return fn(n) { return n * factor; };
    }
    let twice = makeMultiplier(2);
    print twice(5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Lambdas, NestedLambdas) {
  std::string source = R"(
    let f = fn(a) {
      return fn(b) {
        return fn(c) {
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
    struct Greeter {
      init(name) {
        self.name = name;
      }
      getGreeter() {
        return fn() { print "Hi, " + self.name; };
      }
    }
    let g = Greeter("Seam");
    let f = g.getGreeter();
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Hi, Seam\n");
}

TEST_F(Lambdas, LambdaCaptureAndShadow) {
  std::string source = R"(
    let x = "outer";
    let f = nil;
    {
      let x = "inner";
      f = fn() { print x; };
    }
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "inner\n");
}

TEST_F(Lambdas, LambdaWithSideEffects) {
  std::string source = R"(
    let result = 0;
    let f = fn() { result = 42; };
    f();
    print result;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Lambdas, LambdaModifiesCapture) {
  std::string source = R"(
    let x = 10;
    let f = fn() { x = x + 5; };
    f();
    f();
    print x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Lambdas, LambdaPrints) {
  std::string source = R"(
    let f = fn() { print "hello from lambda"; };
    f();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello from lambda\n");
}

TEST_F(Lambdas, LambdaInVariable) {
  std::string source = R"(
    let greet = fn() { print "hi"; };
    greet();
    greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hi\nhi\n");
}

TEST_F(Lambdas, LambdaPassedToFunction) {
  std::string source = R"(
    let output = "";
    fn execute(f) {
      f();
    }
    execute(fn() { output = "executed"; });
    print output;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "executed\n");
}
