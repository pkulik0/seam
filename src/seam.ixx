module;

#include <replxx.hxx>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

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
    replxx::Replxx rx;

    const auto history_path =
        std::filesystem::path{std::getenv("HOME")} / ".seam_history";
    rx.history_load(history_path.string());

    rx.set_completion_callback(
        [](const std::string &context, int & /*context_len*/) {
          replxx::Replxx::completions_t completions;
          std::vector<std::string> examples = {
              "exit"}; // TODO: Implement a completion system
          for (const auto &ex : examples) {
            if (ex.rfind(context, 0) == 0) {
              completions.emplace_back(ex);
            }
          }
          return completions;
        });
    rx.set_highlighter_callback(
        [](const std::string &context, replxx::Replxx::colors_t &colors) {
          for (size_t i = 0; i < context.length();
               ++i) { // TODO: Create a highlighter
            if (std::isdigit(context[i])) {
              colors[i] = replxx::Replxx::Color::YELLOW;
            }
          }
        });

    while (true) {
      const char *c_line = rx.input(m_prompt);
      if (c_line == nullptr) {
        std::println("Bye!");
        break;
      }

      const std::string line{c_line};
      if (line.empty()) {
        continue;
      }
      if (line == "exit") {
        std::println("Bye!");
        break;
      }

      rx.history_add(line);
      run(line);
      m_had_error = false;
    }

    rx.history_save(history_path.string());
    std::println(""); // new line after exit
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
                           [](const auto &token) {
                             return token.type() != token::Type::COMMENT;
                           });

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