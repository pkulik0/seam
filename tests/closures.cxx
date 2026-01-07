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

class Closures : public fixtures::Execution {};

TEST_F(Closures, BasicClosure) {
  std::string source = R"(
    fun outer() {
      var x = "captured";
      fun inner() {
        print x;
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
    fun outer() {
      var x = 1;
      fun inner() {
        print x;
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
    fun makeCounter() {
      var count = 0;
      fun increment() {
        count = count + 1;
        return count;
      }
      return increment;
    }
    var counter = makeCounter();
    print counter();
    print counter();
    print counter();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n3\n");
}

TEST_F(Closures, NestedClosures) {
  std::string source = R"(
    fun outer() {
      var a = "a";
      fun middle() {
        var b = "b";
        fun inner() {
          print a;
          print b;
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
    fun makeAdder(x) {
      fun adder(y) {
        return x + y;
      }
      return adder;
    }
    var add5 = makeAdder(5);
    print add5(3);
    print add5(10);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "8\n15\n");
}

TEST_F(Closures, CounterExample) {
  std::string source = R"(
    fun createCounter() {
      var count = 0;
      fun counter() {
        count = count + 1;
        return count;
      }
      return counter;
    }
    var c1 = createCounter();
    var c2 = createCounter();
    print c1();
    print c1();
    print c2();
    print c1();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n1\n3\n");
}

TEST_F(Closures, ClosureModifiesEnclosing) {
  std::string source = R"(
    fun outer() {
      var x = 0;
      fun setX(val) {
        x = val;
      }
      fun getX() {
        return x;
      }
      setX(42);
      print getX();
    }
    outer();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Closures, ClosureWithParameters) {
  std::string source = R"(
    fun makeAdder(x) {
      return fun(y) { return x + y; };
    }
    var add5 = makeAdder(5);
    print add5(10);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "15\n");
}

TEST_F(Closures, CaptureLoopVariable) {
  std::string source = R"(
    var f1 = nil;
    var f2 = nil;
    for (var i = 0; i < 2; i = i + 1) {
      var j = i;
      if (i == 0) f1 = fun() { print j; };
      else f2 = fun() { print j; };
    }
    f1();
    f2();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n1\n");
}

TEST_F(Closures, CaptureForInitializer) {
  std::string source = R"(
    var f1 = nil;
    var f2 = nil;
    for (var i = 0; i < 2; i = i + 1) {
      if (i == 0) f1 = fun() { print i; };
      else f2 = fun() { print i; };
    }
    f1();
    f2();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n2\n");
}
