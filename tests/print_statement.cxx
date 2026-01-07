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

struct PrintStatement : public fixtures::Execution {};

TEST_F(PrintStatement, PrintNumber) {
  std::string source = R"(
    print 42;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(PrintStatement, PrintFloat) {
  std::string source = R"(
    print 3.14;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "3.14\n");
}

TEST_F(PrintStatement, PrintString) {
  std::string source = R"(
    print "hello";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "hello\n");
}

TEST_F(PrintStatement, PrintTrue) {
  std::string source = R"(
    print true;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "true\n");
}

TEST_F(PrintStatement, PrintFalse) {
  std::string source = R"(
    print false;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "false\n");
}

TEST_F(PrintStatement, PrintNil) {
  std::string source = R"(
    print nil;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "nil\n");
}

TEST_F(PrintStatement, PrintExpression) {
  std::string source = R"(
    print 2 + 2;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "4\n");
}

TEST_F(PrintStatement, PrintFunctionResult) {
  std::string source = R"(
    fn double(n) { return n * 2; }
    print double(21);
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "42\n");
}

TEST_F(PrintStatement, MultipleStatements) {
  std::string source = R"(
    print "first";
    print "second";
    print "third";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "first\nsecond\nthird\n");
}

TEST_F(PrintStatement, PrintEmptyString) {
  std::string source = R"(
    print "";
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "\n");
}

TEST_F(PrintStatement, PrintStruct) {
  std::string source = R"(
    struct Foo {}
    print Foo;
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Foo\n");
}

TEST_F(PrintStatement, PrintInstance) {
  std::string source = R"(
    struct Bar {}
    print Bar();
  )";
  EXPECT_NO_THROW(run(source));
  EXPECT_EQ(last_output(), "Bar instance\n");
}
