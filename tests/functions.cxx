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

struct Functions : public fixtures::Execution {};

// Basic Functions

TEST_F(Functions, NoParams) {
  std::string source = R"(
    fn greet() {
      print("hi");
    }
    greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hi\n");
}

TEST_F(Functions, WithParams) {
  std::string source = R"(
    fn add(a, b) {
      print(a + b);
    }
    add(2, 3);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "5\n");
}

TEST_F(Functions, ReturnValue) {
  std::string source = R"(
    fn double(x) {
      return x * 2;
    }
    print(double(5));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}

TEST_F(Functions, ReturnFromBlock) {
  std::string source = R"(
    fn test() {
      {
        return "from block";
      }
    }
    print(test());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "from block\n");
}

TEST_F(Functions, EarlyReturnWithLiteral) {
  std::string source = R"(
    fn earlyExit() {
      if (true) return "early";
      return "late";
    }
    print(earlyExit());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "early\n");
}

TEST_F(Functions, MultipleParams) {
  std::string source = R"(
    fn sum(a, b, c, d, e) {
      return a + b + c + d + e;
    }
    print(sum(1, 2, 3, 4, 5));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "15\n");
}

TEST_F(Functions, MutualRecursion) {
  std::string source = R"(
    fn isEven(n) {
      if (n == 0) return true;
      return isOdd(n - 1);
    }

    fn isOdd(n) {
      if (n == 0) return false;
      return isEven(n - 1);
    }

    print(isEven(4));
    print(isOdd(4));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\nfalse\n");
}

TEST_F(Functions, DeepRecursion) {
  std::string source = R"(
    fn fib(n) {
      if (n <= 1) return n;
      return fib(n - 1) + fib(n - 2);
    }
    print(fib(10));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "55\n");
}

TEST_F(Functions, LocalShadowingInBlock) {
  std::string source = R"(
    fn test(x) {
      {
        let x = "shadow";
        print(x);
      }
      print(x);
    }
    test("original");
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "shadow\noriginal\n");
}

TEST_F(Functions, DuplicateParamError) {
  std::string source = R"(
    fn test(x, x) {
      print(x);
    }
  )";
  EXPECT_THROW(run(source), Error);
}

// Function Scope

TEST_F(Functions, RecursiveClosureShadowing) {
  std::string source = R"(
    fn makeCounter(x) {
      if (x <= 0) return fn() { return 0; };
      let next = makeCounter(x - 1);
      return fn() {
        let y = x;
        return y + next();
      };
    }
    let sum = makeCounter(5);
    print(sum());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "15\n");
}

TEST_F(Functions, ShadowingGlobal) {
  std::string source = R"(
    let x = "global";
    fn test(x) {
      print(x);
    }
    test("param");
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "param\nglobal\n");
}

TEST_F(Functions, AccessGlobal) {
  std::string source = R"(
    let x = "global";
    fn test() {
      print(x);
    }
    test();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "global\n");
}

TEST_F(Functions, ModifyGlobal) {
  std::string source = R"(
    let x = 1;
    fn increment() {
      x = x + 1;
    }
    increment();
    increment();
    print(x);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n");
}

// Error Cases

TEST_F(Functions, WrongArity) {
  std::string source = R"(
    fn add(a, b) {
      return a + b;
    }
    add(1);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Functions, TooManyArgs) {
  std::string source = R"(
    fn add(a, b) {
      return a + b;
    }
    add(1, 2, 3);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Functions, CallNonFunction) {
  std::string source = R"(
    let x = 5;
    x();
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Functions, UndefinedFunction) {
  std::string source = R"(
    undefinedFunctionc();
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Functions, ReturnOutsideFunction) {
  std::string source = R"(
    return 5;
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Functions, FunctionAsValue) {
  std::string source = R"(
    fn greet() {
      return "hello";
    }
    let f = greet;
    print(f());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(Functions, PassFunctionAsArgument) {
  std::string source = R"(
    fn apply(f, x) {
      return f(x);
    }
    fn double(n) {
      return n * 2;
    }
    print(apply(double, 5));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n");
}
