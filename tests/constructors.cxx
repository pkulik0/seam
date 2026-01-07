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

class Constructors : public fixtures::Execution {};

TEST_F(Constructors, BasicInit) {
  std::string source = R"(
    class Point {
      init(x, y) {
        this.x = x;
        this.y = y;
      }
    }
    var p = Point(3, 4);
    print p.x;
    print p.y;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n4\n");
}

TEST_F(Constructors, InitNoParams) {
  std::string source = R"(
    class Counter {
      init() {
        this.count = 0;
      }
    }
    var c = Counter();
    print c.count;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n");
}

TEST_F(Constructors, InitMultipleParams) {
  std::string source = R"(
    class Rectangle {
      init(x, y, width, height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
      }
      area() {
        return this.width * this.height;
      }
    }
    var r = Rectangle(0, 0, 10, 5);
    print r.area();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "50\n");
}

TEST_F(Constructors, InitReturnsInstance) {
  std::string source = R"(
    class Box {
      init(value) {
        this.value = value;
      }
    }
    var b = Box(42);
    print b;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Box instance\n");
}

TEST_F(Constructors, InitWithMultipleStatements) {
  std::string source = R"(
    class Point {
      init(x, y) {
        this.x = x;
        this.y = y;
        this.sum = x + y;
      }
    }
    var p = Point(3, 4);
    print p.sum;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "7\n");
}

TEST_F(Constructors, InitCallsMethod) {
  std::string source = R"(
    class Circle {
      init(radius) {
        this.radius = radius;
        this.calculateArea();
      }
      calculateArea() {
        this.area = 3.14 * this.radius * this.radius;
      }
    }
    var c = Circle(2);
    print c.area;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "12.56\n");
}

TEST_F(Constructors, InitWithStringParam) {
  std::string source = R"(
    class Greeter {
      init(name) {
        this.greeting = "Hello, " + name;
      }
    }
    var g = Greeter("World");
    print g.greeting;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Hello, World\n");
}

TEST_F(Constructors, InitSetsMultipleProperties) {
  std::string source = R"(
    class Person {
      init(first, last) {
        this.firstName = first;
        this.lastName = last;
        this.fullName = first + " " + last;
      }
    }
    var p = Person("John", "Doe");
    print p.firstName;
    print p.lastName;
    print p.fullName;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "John\nDoe\nJohn Doe\n");
}

// Error Cases

TEST_F(Constructors, InitWrongArity) {
  std::string source = R"(
    class Point {
      init(x, y) {
        this.x = x;
        this.y = y;
      }
    }
    var p = Point(1);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, InitTooManyArgs) {
  std::string source = R"(
    class Point {
      init(x, y) {
        this.x = x;
        this.y = y;
      }
    }
    var p = Point(1, 2, 3);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, ThisOutsideClass) {
  std::string source = R"(
    print this;
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, ThisInFunction) {
  std::string source = R"(
    fun test() {
      print this;
    }
    test();
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, InitExplicitReturnThis) {
  std::string source = R"(
    class Box {
      init(value) {
        this.value = value;
        return;
      }
    }
    var b = Box(42);
    print b.value;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Constructors, InitEarlyReturnReturnsThis) {
  std::string source = R"(
    class Box {
      init(value) {
        this.value = value;
        if (value == "early") return;
        this.value = "late";
      }
    }

    print Box("early").value;
    print Box("other").value;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "early\nlate\n");
}
