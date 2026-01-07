module;

#include <gtest/gtest.h>
#include <sstream>
#include <string>

export module seam.tests.fixtures;

import seam.interpreter;
import seam.resolver;
import seam.scanner;
import seam.parser;
import seam.token;

using namespace seam;

export namespace seam::tests::fixtures {

struct Execution : public ::testing::Test {
protected:
  std::stringstream m_out;
  interpreter::Interpreter interpreter{m_out};
  resolver::Resolver resolver{interpreter};

  std::string last_output() const { return m_out.str(); }

  void run(const std::string &source) {
    m_out.str("");
    m_out.clear();
    scanner::Scanner scanner{source};
    auto tokens = scanner.scan_tokens();

    std::vector<token::Token> filtered_tokens;
    for (const auto &token : tokens) {
      if (token.type() != token::Type::COMMENT) {
        filtered_tokens.push_back(token);
      }
    }

    parser::Parser parser{filtered_tokens};
    auto program = parser.parse();
    resolver.resolve(program);
    interpreter.execute(program);
  }
};

} // namespace seam::tests::fixtures