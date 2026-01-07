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

struct StaticMethods : public fixtures::Execution {};

TEST_F(StaticMethods, CanCallStaticMethod) {
  std::string source = R"(
        struct Math {
            static add(a, b) {
                return a + b;
            }
        }
        print Math.add(5, 10);
    )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "15\n");
}

TEST_F(StaticMethods, StaticMethodCannotUseThis) {
  std::string source = R"(
        struct Math {
            static fail() {
                print self;
            }
        }
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(StaticMethods, CannotCallStaticMethodOnInstance) {
  std::string source = R"(
        struct Math {
            static add(a, b) {
                return a + b;
            }
        }
        let m = Math();
        print m.add(1, 2);
    )";
  EXPECT_THROW(run(source), Error);
}

TEST_F(StaticMethods, CannotCallInstanceMethodOnStruct) {
  std::string source = R"(
        struct Math {
            add(a, b) {
                return a + b;
            }
        }
        print Math.add(1, 2);
    )";
  EXPECT_THROW(run(source), Error);
}
