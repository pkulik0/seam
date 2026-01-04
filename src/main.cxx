#include <print>

import seam;

int main(int argc, char** argv) {
  seam::Seam seam{};
  if(argc > 2) {
    std::println("Usage: {} <path_to_script>", argv[0]);
    return 1;
  } else if(argc == 2) {
    return seam.run_file(argv[1]);
  } else {
    seam.run_repl();
    return 0;
  }
}
