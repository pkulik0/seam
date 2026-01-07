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

struct Inheritance : public fixtures::Execution {};

TEST_F(Inheritance, InheritMethod) {
  std::string source = R"(
    struct Animal {
      speak() {
        print "sound";
      }
    }
    struct Dog + Animal {}
    let d = Dog();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "sound\n");
}

TEST_F(Inheritance, OverrideMethod) {
  std::string source = R"(
    struct Animal {
      speak() {
        print "generic sound";
      }
    }
    struct Dog + Animal {
      speak() {
        print "bark";
      }
    }
    let d = Dog();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "bark\n");
}

TEST_F(Inheritance, InheritMultipleMethods) {
  std::string source = R"(
    struct Shape {
      getType() {
        return "shape";
      }
      describe() {
        print "I am a shape";
      }
    }
    struct Circle + Shape {}
    let c = Circle();
    print c.getType();
    c.describe();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "shape\nI am a shape\n");
}

TEST_F(Inheritance, InheritedMethodAccessesThis) {
  std::string source = R"(
    struct Animal {
      getName() {
        return self.name;
      }
    }
    struct Dog + Animal {}
    let d = Dog();
    d.name = "Rex";
    print d.getName();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Rex\n");
}

TEST_F(Inheritance, MultiLevelInheritance) {
  std::string source = R"(
    struct A {
      methodA() {
        print "A";
      }
    }
    struct B + A {
      methodB() {
        print "B";
      }
    }
    struct C + B {
      methodC() {
        print "C";
      }
    }
    let c = C();
    c.methodA();
    c.methodB();
    c.methodC();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "A\nB\nC\n");
}

TEST_F(Inheritance, OverrideInChain) {
  std::string source = R"(
    struct A {
      greet() {
        print "A";
      }
    }
    struct B + A {
      greet() {
        print "B";
      }
    }
    struct C + B {}
    let c = C();
    c.greet();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "B\n");
}

TEST_F(Inheritance, ChildCanAddMethods) {
  std::string source = R"(
    struct Parent {
      parentMethod() {
        print "parent";
      }
    }
    struct Child + Parent {
      childMethod() {
        print "child";
      }
    }
    let c = Child();
    c.parentMethod();
    c.childMethod();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "parent\nchild\n");
}

TEST_F(Inheritance, ChildInitCallsOwnMethods) {
  std::string source = R"(
    struct Parent {
      setup() {
        self.value = 10;
      }
    }
    struct Child + Parent {
      init() {
        self.setup();
        self.value = self.value * 2;
      }
    }
    let c = Child();
    print c.value;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "20\n");
}

TEST_F(Inheritance, ParentAndChildInstances) {
  std::string source = R"(
    struct Animal {
      speak() {
        print "animal sound";
      }
    }
    struct Dog + Animal {
      speak() {
        print "bark";
      }
    }
    let a = Animal();
    let d = Dog();
    a.speak();
    d.speak();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "animal sound\nbark\n");
}

TEST_F(Inheritance, ChildMethodCallsInheritedMethod) {
  std::string source = R"(
    struct A {
      method() {
        print "A";
      }
    }
    struct B + A {
      method() {
        print "B";
        parent.method();
      }
    }
    let b = B();
    b.method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "B\nA\n");
}

TEST_F(Inheritance, DeepInheritanceWithSuper) {
  std::string source = R"(
    struct A {
      method() {
        print "A";
      }
    }

    struct B + A {
      method() {
        print "B";
        parent.method();
      }
    }

    struct C + B {
      method() {
        print "C";
        parent.method();
      }
    }

    let c = C();
    c.method();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "C\nB\nA\n");
}
