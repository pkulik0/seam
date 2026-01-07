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

class Classes : public fixtures::Execution {};

// Class Declaration and Instantiation

TEST_F(Classes, EmptyClass) {
  std::string source = R"(
    class Foo {}
    var f = Foo();
    print f;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Foo instance\n");
}

TEST_F(Classes, ClassAsValue) {
  std::string source = R"(
    class Foo {}
    var cls = Foo;
    var instance = cls();
    print instance;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Foo instance\n");
}

TEST_F(Classes, MultipleInstances) {
  std::string source = R"(
    class Counter {}
    var a = Counter();
    var b = Counter();
    a.count = 1;
    b.count = 2;
    print a.count;
    print b.count;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "1\n2\n");
}

// Instance Properties

TEST_F(Classes, SetProperty) {
  std::string source = R"(
    class Obj {}
    var o = Obj();
    o.x = 5;
    print o.x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "5\n");
}

TEST_F(Classes, PropertyIndependence) {
  std::string source = R"(
    class Point {}
    var p1 = Point();
    var p2 = Point();
    p1.x = 10;
    p1.y = 20;
    p2.x = 30;
    p2.y = 40;
    print p1.x;
    print p2.x;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "10\n30\n");
}

TEST_F(Classes, GetUndefinedPropertyError) {
  std::string source = R"(
    class Obj {}
    var o = Obj();
    print o.undefined;
  )";
  EXPECT_THROW(run(source), Error);
}

// Instance Methods

TEST_F(Classes, BasicMethod) {
  std::string source = R"(
    class Greeter {
      greet() {
        print "hello";
      }
    }
    var g = Greeter();
    g.greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(Classes, MethodWithParams) {
  std::string source = R"(
    class Math {
      add(a, b) {
        return a + b;
      }
    }
    var m = Math();
    print m.add(3, 4);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "7\n");
}

TEST_F(Classes, MethodReturnsValue) {
  std::string source = R"(
    class Calculator {
      square(n) {
        return n * n;
      }
    }
    var c = Calculator();
    print c.square(5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "25\n");
}

TEST_F(Classes, MethodAccessesThis) {
  std::string source = R"(
    class Person {
      getName() {
        return this.name;
      }
    }
    var p = Person();
    p.name = "Alice";
    print p.getName();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Alice\n");
}

TEST_F(Classes, MethodModifiesThis) {
  std::string source = R"(
    class Counter {
      increment() {
        this.count = this.count + 1;
      }
    }
    var c = Counter();
    c.count = 0;
    c.increment();
    c.increment();
    print c.count;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "2\n");
}

TEST_F(Classes, ChainedMethodCalls) {
  std::string source = R"(
    class Builder {
      setX(x) {
        this.x = x;
        return this;
      }
      setY(y) {
        this.y = y;
        return this;
      }
      build() {
        return this.x + this.y;
      }
    }
    var b = Builder();
    print b.setX(10).setY(20).build();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "30\n");
}

TEST_F(Classes, MethodCallsOtherMethod) {
  std::string source = R"(
    class Math {
      double(n) {
        return n * 2;
      }
      quadruple(n) {
        return this.double(this.double(n));
      }
    }
    var m = Math();
    print m.quadruple(5);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Classes, MultipleMethods) {
  std::string source = R"(
    class Account {
      deposit(amount) {
        this.balance = this.balance + amount;
      }
      withdraw(amount) {
        this.balance = this.balance - amount;
      }
      getBalance() {
        return this.balance;
      }
    }
    var acc = Account();
    acc.balance = 100;
    acc.deposit(50);
    acc.withdraw(30);
    print acc.getBalance();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "120\n");
}

TEST_F(Classes, FluentInterface) {
  std::string source = R"(
    class Calc {
      init() { this.value = 0; }
      add(n) { this.value = this.value + n; return this; }
      mul(n) { this.value = this.value * n; return this; }
    }
    print Calc().add(5).mul(3).add(2).value;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "17\n");
}

TEST_F(Classes, PropertyShadowsMethod) {
  std::string source = R"(
    class A {
      m() { print "method"; }
    }
    var a = A();
    a.m = "property";
    print a.m;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "property\n");
}
