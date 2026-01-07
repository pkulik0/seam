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

class Super : public fixtures::Execution {};

TEST_F(Super, BasicInheritance) {
  std::string source = R"(
        class A {
            method() {
                return "A";
            }
        }
        class B + A {
            method() {
                return "B" + super.method();
            }
        }
        var b = B();
        print b.method();
    )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "BA\n");
}

TEST_F(Super, InConstructor) {
  std::string source = R"(
        class A {
            init(a) {
                this.a = a;
            }
        }
        class B + A {
            init(a, b) {
                super.init(a);
                this.b = b;
            }
        }
        var b = B("a", "b");
        print b.a;
        print b.b;
    )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\n");
}

TEST_F(Super, WithoutSuperclass) {
  std::string source = R"(
        class A {
            method() {
                print super.method();
            }
        }
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Super, OutsideClass) {
  std::string source = R"(
        print super.method();
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Super, MethodDoesNotExist) {
  std::string source = R"(
        class A {}
        class B + A {
            method() {
                super.noMethod();
            }
        }
        var b = B();
        b.method();
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Super, InheritFromNonClass) {
  std::string source = R"(
        var NotAClass = "I am a string";
        class B + NotAClass {}
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Super, InheritFromSelf) {
  std::string source = R"(
        class A + A {}
    )";
  EXPECT_THROW(run(source), Error);
}
