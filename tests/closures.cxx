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

struct Closures : public fixtures::Execution {};

TEST_F(Closures, BasicClosure) {
  std::string source = R"(
    fn outer() {
      let x = "captured";
      fn inner() {
        print(x);
      }
      inner();
    }
    outer();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "captured\n");
}

TEST_F(Closures, CloseOverMutable) {
  std::string source = R"(
    fn outer() {
      let x = 1;
      fn inner() {
        print(x);
      }
      inner();
      x = 2;
      inner();
    }
    outer();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n");
}

TEST_F(Closures, MultipleClosuresShareState) {
  std::string source = R"(
    fn makeCounter() {
      let count = 0;
      fn increment() {
        count = count + 1;
        return count;
      }
      return increment;
    }
    let counter = makeCounter();
    print(counter());
    print(counter());
    print(counter());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n3\n");
}

TEST_F(Closures, NestedClosures) {
  std::string source = R"(
    fn outer() {
      let a = "a";
      fn middle() {
        let b = "b";
        fn inner() {
          print(a);
          print(b);
        }
        inner();
      }
      middle();
    }
    outer();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\n");
}

TEST_F(Closures, ClosureReturnedFromFunction) {
  std::string source = R"(
    fn makeAdder(x) {
      fn adder(y) {
        return x + y;
      }
      return adder;
    }
    let add5 = makeAdder(5);
    print(add5(3));
    print(add5(10));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "8\n15\n");
}

TEST_F(Closures, CounterExample) {
  std::string source = R"(
    fn createCounter() {
      let count = 0;
      fn counter() {
        count = count + 1;
        return count;
      }
      return counter;
    }
    let c1 = createCounter();
    let c2 = createCounter();
    print(c1());
    print(c1());
    print(c2());
    print(c1());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n1\n3\n");
}

TEST_F(Closures, ClosureModifiesEnclosing) {
  std::string source = R"(
    fn outer() {
      let x = 0;
      fn setX(val) {
        x = val;
      }
      fn getX() {
        return x;
      }
      setX(42);
      print(getX());
    }
    outer();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Closures, ClosureWithParameters) {
  std::string source = R"(
    fn makeAdder(x) {
      return fn(y) { return x + y; };
    }
    let add5 = makeAdder(5);
    print(add5(10));
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "15\n");
}

TEST_F(Closures, CaptureLoopVariable) {
  std::string source = R"(
    let f1 = nil;
    let f2 = nil;
    for (let i = 0; i < 2; i = i + 1) {
      let j = i;
      if (i == 0) f1 = fn() { print(j); };
      else f2 = fn() { print(j); };
    }
    f1();
    f2();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n");
}

TEST_F(Closures, CaptureForInitializer) {
  std::string source = R"(
    let f1 = nil;
    let f2 = nil;
    for (let i = 0; i < 2; i = i + 1) {
      if (i == 0) f1 = fn() { print(i); };
      else f2 = fn() { print(i); };
    }
    f1();
    f2();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n2\n");
}
