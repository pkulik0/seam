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

struct Constructors : public fixtures::Execution {};

TEST_F(Constructors, BasicInit) {
  std::string source = R"(
    struct Point {
      init(x, y) {
        self.x = x;
        self.y = y;
      }
    }
    let p = Point(3, 4);
    print(p.x);
    print(p.y);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3\n4\n");
}

TEST_F(Constructors, InitNoParams) {
  std::string source = R"(
    struct Counter {
      init() {
        self.count = 0;
      }
    }
    let c = Counter();
    print(c.count);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "0\n");
}

TEST_F(Constructors, InitMultipleParams) {
  std::string source = R"(
    struct Rectangle {
      init(x, y, width, height) {
        self.x = x;
        self.y = y;
        self.width = width;
        self.height = height;
      }
      area() {
        return self.width * self.height;
      }
    }
    let r = Rectangle(0, 0, 10, 5);
    print(r.area());
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "50\n");
}

TEST_F(Constructors, InitReturnsInstance) {
  std::string source = R"(
    struct Box {
      init(value) {
        self.value = value;
      }
    }
    let b = Box(42);
    print(b);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Box instance\n");
}

TEST_F(Constructors, InitWithMultipleStatements) {
  std::string source = R"(
    struct Point {
      init(x, y) {
        self.x = x;
        self.y = y;
        self.sum = x + y;
      }
    }
    let p = Point(3, 4);
    print(p.sum);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "7\n");
}

TEST_F(Constructors, InitCallsMethod) {
  std::string source = R"(
    struct Circle {
      init(radius) {
        self.radius = radius;
        self.calculateArea();
      }
      calculateArea() {
        self.area = 3.14 * self.radius * self.radius;
      }
    }
    let c = Circle(2);
    print(c.area);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "12.56\n");
}

TEST_F(Constructors, InitWithStringParam) {
  std::string source = R"(
    struct Greeter {
      init(name) {
        self.greeting = "Hello, " + name;
      }
    }
    let g = Greeter("World");
    print(g.greeting);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Hello, World\n");
}

TEST_F(Constructors, InitSetsMultipleProperties) {
  std::string source = R"(
    struct Person {
      init(first, last) {
        self.firstName = first;
        self.lastName = last;
        self.fullName = first + " " + last;
      }
    }
    let p = Person("John", "Doe");
    print(p.firstName);
    print(p.lastName);
    print(p.fullName);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "John\nDoe\nJohn Doe\n");
}

TEST_F(Constructors, InitWrongArity) {
  std::string source = R"(
    struct Point {
      init(x, y) {
        self.x = x;
        self.y = y;
      }
    }
    let p = Point(1);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, InitTooManyArgs) {
  std::string source = R"(
    struct Point {
      init(x, y) {
        self.x = x;
        self.y = y;
      }
    }
    let p = Point(1, 2, 3);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, ThisOutsideStruct) {
  std::string source = R"(
    print(self);
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, ThisInFn) {
  std::string source = R"(
    fn test() {
      print(self);
    }
    test();
  )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Constructors, InitExplicitReturnThis) {
  std::string source = R"(
    struct Box {
      init(value) {
        self.value = value;
        return;
      }
    }
    let b = Box(42);
    print(b.value);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(Constructors, InitEarlyReturnReturnsThis) {
  std::string source = R"(
    struct Box {
      init(value) {
        self.value = value;
        if (value == "early") return;
        self.value = "late";
      }
    }

    print(Box("early").value);
    print(Box("other").value);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "early\nlate\n");
}
