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

class NativeFunctions : public fixtures::Execution {};

TEST_F(NativeFunctions, ClockReturnsNumber) {
  std::string source = R"(
    var t = clock();
    print t >= 0;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(NativeFunctions, ClockCanBeUsedInExpressions) {
  std::string source = R"(
    var start = clock();
    var i = 0;
    while (i < 1000) {
      i = i + 1;
    }
    var end = clock();
    print end >= start;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(NativeFunctions, ClockCallable) {
  std::string source = R"(
    var f = clock;
    print f() >= 0;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}
