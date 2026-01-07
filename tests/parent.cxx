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

struct Parent : public fixtures::Execution {};

TEST_F(Parent, BasicInheritance) {
  std::string source = R"(
        struct A {
            method() {
                return "A";
            }
        }
        struct B + A {
            method() {
                return "B" + parent.method();
            }
        }
        let b = B();
        print b.method();
    )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "BA\n");
}

TEST_F(Parent, InConstructor) {
  std::string source = R"(
        struct A {
            init(a) {
                self.a = a;
            }
        }
        struct B + A {
            init(a, b) {
                parent.init(a);
                self.b = b;
            }
        }
        let b = B("a", "b");
        print b.a;
        print b.b;
    )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "a\nb\n");
}

TEST_F(Parent, WithoutSuperstruct) {
  std::string source = R"(
        struct A {
            method() {
                print parent.method();
            }
        }
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Parent, OutsideStruct) {
  std::string source = R"(
        print parent.method();
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Parent, MethodDoesNotExist) {
  std::string source = R"(
        struct A {}
        struct B + A {
            method() {
                parent.noMethod();
            }
        }
        let b = B();
        b.method();
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Parent, InheritFromNonStruct) {
  std::string source = R"(
        let NotAStruct = "I am a string";
        struct B + NotAStruct {}
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(Parent, InheritFromSelf) {
  std::string source = R"(
        struct A + A {}
    )";
  EXPECT_THROW(run(source), Error);
}