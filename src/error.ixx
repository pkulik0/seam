module;

#include <stdexcept>
#include <string>

export module seam.error;

export namespace seam {

class Error : public std::exception {
public:
  struct Location {
    int line{0};
    int column{0};
  };

  ~Error() override = default;

  [[nodiscard]] virtual auto name() const noexcept -> const char * = 0;
  [[nodiscard]] virtual auto reason() const noexcept -> const char * = 0;
  [[nodiscard]] virtual auto location() const noexcept -> Location = 0;

  [[nodiscard]] auto what() const noexcept -> const char * override {
    return reason();
  }
};
} // namespace seam
