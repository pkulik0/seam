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

class StaticMethods : public fixtures::Execution {};

TEST_F(StaticMethods, CanCallStaticMethod) {
  std::string source = R"(
        class Math {
            static add(a, b) {
                return a + b;
            }
        }
        print Math.add(5, 10);
    )";
  EXPECT_NO_THROW(run(source));
}

TEST_F(StaticMethods, StaticMethodCannotUseThis) {
  std::string source = R"(
        class Math {
            static fail() {
                print this;
            }
        }
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(StaticMethods, CannotCallStaticMethodOnInstance) {
  std::string source = R"(
        class Math {
            static add(a, b) {
                return a + b;
            }
        }
        var m = Math();
        print m.add(1, 2);
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(StaticMethods, CannotCallInstanceMethodOnClass) {
  std::string source = R"(
        class Math {
            add(a, b) {
                return a + b;
            }
        }
        print Math.add(1, 2);
    )";
  EXPECT_THROW(run(source), Error);
}
