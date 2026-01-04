module;

#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

export module seam;

import seam.scanner;
import seam.common;

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
    if (m_had_error) return 2;
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

  void run(const std::string_view source) {
    scanner::Scanner scanner{source};
  }

  void error(usize line, usize column, const std::string_view message) {
    std::println("Error: {} (line {}, column {})", message, line, column);
    m_had_error = true;
  }
};

} // namespace seam