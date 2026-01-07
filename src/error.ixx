module;

#include <stdexcept>
#include <string>
#include <string_view>

export module seam.error;

export namespace seam {

class Error : public std::exception {
public:
  struct Location {
    int line{0};
    int column{0};
  };

  ~Error() override = default;

  [[nodiscard]] virtual auto name() const noexcept -> std::string_view= 0;
  [[nodiscard]] virtual auto reason() const noexcept -> std::string_view = 0;
  [[nodiscard]] virtual auto location() const noexcept -> Location = 0;

  [[nodiscard]] auto what() const noexcept -> const char * override {
    return reason().data();
  }
};
} // namespace seam
