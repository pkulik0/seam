#include <CLI/CLI.hpp>
#include <print>

import seam;

int main(int argc, char **argv) {
  seam::Options options{};

  CLI::App app{"Seam Programming Language"};
  app.require_subcommand(0, 1);
  app.footer("If no subcommand is provided, the REPL (interactive mode) will start.");
  app.add_flag("-v,--verbose", options.is_verbose, "Verbose output");

  auto *run_cmd = app.add_subcommand("run", "Run a Seam script");
  run_cmd->add_option("file", options.script_path, "Path to a Seam script")->required();
  run_cmd->add_flag("-v,--verbose", options.is_verbose, "Verbose output");

  auto *version_cmd = app.add_subcommand("version", "Print version information");
  CLI11_PARSE(app, argc, argv);

  seam::Seam seam{options};

  if (*version_cmd) {
    seam.print_version();
    return 0;
  } else if (*run_cmd) {
    return seam.run_script();
  } else {
    seam.run_repl();
    return 0;
  }
}
