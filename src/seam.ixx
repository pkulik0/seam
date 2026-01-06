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

export namespace seam {

class Seam {
public:
  Seam() = default;

  int run_file(const std::filesystem::path &path) {
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
    std::println("Welcome to Seam {}", VERSION);
    std::string buffer{};
    while (true) {
      std::print("{}", m_prompt);
      std::getline(std::cin, buffer);
      run(buffer);
      m_had_error = false;
    }
  }

private:
  static constexpr std::string_view VERSION = "0.1.0-dev";

  std::string m_prompt = ">> ";
  bool m_had_error = false;

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
      std::println("\t{}", m_ast_printer.print(program));

      m_resolver.resolve(program);
      m_interpreter.run(program);
    } catch (const scanner::Error &e) {
      std::println("{}", e.what());
      m_had_error = true;
    } catch (const parser::Error &e) {
      std::println("{}", e.what());
      m_had_error = true;
    } catch (const resolver::ResolutionError &e) {
      std::println("{}", e.what());
      m_had_error = true;
    } catch (const interpreter::RuntimeError &e) {
      std::println("{}", e.what());
      m_had_error = true;
    }
  }

  void error(usize line, usize column, const std::string_view message) {
    std::println("Error: {} (line {}, column {})", message, line, column);
    m_had_error = true;
  }
};

} // namespace seam