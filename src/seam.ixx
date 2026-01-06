module;

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>

export module seam;

import seam.scanner;
import seam.common;
import seam.parser;
import seam.ast;
import seam.interpreter;
import seam.resolver;
import seam.token;
import seam.version;

export namespace seam {

struct Options {
  std::optional<std::filesystem::path> script_path{};
  bool is_verbose{false}; // -v --verbose
};

class Seam {
public:
  Seam(Options options) : m_options(std::move(options)) {}

  int run_script() {
    if (!m_options.script_path) {
      std::println("No script path provided");
      return 1;
    }
    const auto path = m_options.script_path.value();
    std::ifstream file{path};
    if (!file.is_open()) {
      std::println("Could not open file \"{}\"", path.string());
      return 1;
    }
    const auto source = std::string{std::istreambuf_iterator<char>{file},
                                    std::istreambuf_iterator<char>{}};
    run(source);
    if (m_had_error)
      return 2;
    return 0;
  }

  void run_repl() {
    std::println("Welcome to Seam {}", version::git_tag);
    std::string buffer{};
    while (true) {
      std::print("{}", m_prompt);
      std::getline(std::cin, buffer);
      run(buffer);
      m_had_error = false;
    }
  }

  void print_version() {
    std::println("Seam {}", version::git_tag);
    std::println("Build time: {}\n", version::build_time);
  }

private:
  std::string m_prompt{">> "};
  bool m_had_error{false};
  Options m_options;

  interpreter::Interpreter m_interpreter;
  resolver::Resolver m_resolver{m_interpreter};
  ast::AstPrinter m_ast_printer;

  void run(const std::string_view source) {
    try {
      scanner::Scanner scanner{source};
      const auto tokens = scanner.scan_tokens();

      std::vector<token::Token> filtered_tokens;
      std::ranges::copy_if(tokens, std::back_inserter(filtered_tokens),
        [](const auto& token) { return token.type() != token::Type::COMMENT; });

      parser::Parser parser{filtered_tokens};
      const auto program = parser.parse();
      if (m_options.is_verbose) {
        std::println("\t{}", m_ast_printer.print(program));
      }

      m_resolver.resolve(program);
      m_interpreter.execute(program);
    } catch (const std::exception &e) {
      std::println("{}", e.what());
      m_had_error = true;
    }
  }
};

} // namespace seam