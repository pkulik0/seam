module;

#include <stdexcept>
#include <string>

export module seam.error;

export namespace seam::error {

struct Location {
	int line{0};
	int column{0};
};

class SeamError : public std::exception {
public:
	~SeamError() override = default;

	[[nodiscard]] virtual auto name() const noexcept -> const char* = 0;
	[[nodiscard]] virtual auto reason() const noexcept -> const char* = 0;
	[[nodiscard]] virtual auto location() const noexcept -> Location = 0;

	[[nodiscard]] auto what() const noexcept -> const char* override {
		return reason();
	}
};
} // namespace seam::error
