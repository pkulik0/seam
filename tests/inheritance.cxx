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

class Inheritance : public fixtures::Execution {};

TEST_F(Inheritance, InheritMethod) {
  std::string source = R"(
    class Animal {
      speak() {
        print "sound";
      }
    }
    class Dog + Animal {}
    var d = Dog();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "sound\n");
}

TEST_F(Inheritance, OverrideMethod) {
  std::string source = R"(
    class Animal {
      speak() {
        print "generic sound";
      }
    }
    class Dog + Animal {
      speak() {
        print "bark";
      }
    }
    var d = Dog();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "bark\n");
}

TEST_F(Inheritance, InheritMultipleMethods) {
  std::string source = R"(
    class Shape {
      getType() {
        return "shape";
      }
      describe() {
        print "I am a shape";
      }
    }
    class Circle + Shape {}
    var c = Circle();
    print c.getType();
    c.describe();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "shape\nI am a shape\n");
}

TEST_F(Inheritance, InheritedMethodAccessesThis) {
  std::string source = R"(
    class Animal {
      getName() {
        return this.name;
      }
    }
    class Dog + Animal {}
    var d = Dog();
    d.name = "Rex";
    print d.getName();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Rex\n");
}

TEST_F(Inheritance, MultiLevelInheritance) {
  std::string source = R"(
    class A {
      methodA() {
        print "A";
      }
    }
    class B + A {
      methodB() {
        print "B";
      }
    }
    class C + B {
      methodC() {
        print "C";
      }
    }
    var c = C();
    c.methodA();
    c.methodB();
    c.methodC();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "A\nB\nC\n");
}

TEST_F(Inheritance, OverrideInChain) {
  std::string source = R"(
    class A {
      greet() {
        print "A";
      }
    }
    class B + A {
      greet() {
        print "B";
      }
    }
    class C + B {}
    var c = C();
    c.greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "B\n");
}

TEST_F(Inheritance, ChildCanAddMethods) {
  std::string source = R"(
    class Parent {
      parentMethod() {
        print "parent";
      }
    }
    class Child + Parent {
      childMethod() {
        print "child";
      }
    }
    var c = Child();
    c.parentMethod();
    c.childMethod();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "parent\nchild\n");
}

TEST_F(Inheritance, ChildInitCallsOwnMethods) {
  std::string source = R"(
    class Parent {
      setup() {
        this.value = 10;
      }
    }
    class Child + Parent {
      init() {
        this.setup();
        this.value = this.value * 2;
      }
    }
    var c = Child();
    print c.value;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Inheritance, ParentAndChildInstances) {
  std::string source = R"(
    class Animal {
      speak() {
        print "animal sound";
      }
    }
    class Dog + Animal {
      speak() {
        print "bark";
      }
    }
    var a = Animal();
    var d = Dog();
    a.speak();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "animal sound\nbark\n");
}

TEST_F(Inheritance, ChildMethodCallsInheritedMethod) {
  std::string source = R"(
    class A {
      method() {
        print "A";
      }
    }
    class B + A {
      method() {
        print "B";
        super.method();
      }
    }
    var b = B();
    b.method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "B\nA\n");
}

TEST_F(Inheritance, DeepInheritanceWithSuper) {
  std::string source = R"(
    class A {
      method() {
        print "A";
      }
    }

    class B + A {
      method() {
        print "B";
        super.method();
      }
    }

    class C + B {
      method() {
        print "C";
        super.method();
      }
    }

    var c = C();
    c.method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "C\nB\nA\n");
}
