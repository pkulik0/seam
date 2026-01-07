module;

#include <any>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

export module seam.runtime;

import seam.token;
import seam.error;

using namespace seam::token;

export namespace seam::runtime {

class RuntimeError : public Error {
public:
  RuntimeError(Token token, std::string message)
      : m_token(std::move(token)), m_message(std::move(message)) {}

  [[nodiscard]] auto name() const noexcept -> std::string_view override {
    return "Runtime Error";
  }
  [[nodiscard]] auto reason() const noexcept -> std::string_view override {
    return m_message;
  }
  [[nodiscard]] auto location() const noexcept -> Location override {
    return {static_cast<int>(m_token.line()),
            static_cast<int>(m_token.column())};
  }

private:
  Token m_token;
  std::string m_message;
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
  Environment() = default;
  Environment(std::shared_ptr<Environment> parent) : m_enclosing(parent) {}

  void define(const Token &name, const std::any &value) {
    m_values[std::string{name.lexeme()}] = value;
  }

  void define(const std::string &name, const std::any &value) {
    m_values[name] = value;
  }

  void assign(const Token &name, const std::any &value) {
    const auto it = m_values.find(std::string{name.lexeme()});
    if (it != m_values.end()) {
      it->second = value;
      return;
    }
    if (m_enclosing) {
      m_enclosing->assign(name, value);
      return;
    }
    throw RuntimeError(name,
                       std::format("Undefined variable '{}'.", name.lexeme()));
  }

  std::optional<std::any> get(const std::string &name) const {
    const auto it = m_values.find(name);
    if (it != m_values.end()) {
      return it->second;
    }
    if (m_enclosing) {
      return m_enclosing->get(name);
    }
    return std::nullopt;
  }

  std::shared_ptr<Environment> ancestor(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this();
    for (int i = 0; i < distance; i++) {
      environment = environment->m_enclosing;
    }
    return environment;
  }

  std::any getAt(int distance, const std::string &name) {
    return ancestor(distance)->m_values[name];
  }

  void assignAt(int distance, const Token &name, const std::any &value) {
    ancestor(distance)->m_values[std::string{name.lexeme()}] = value;
  }

  std::shared_ptr<Environment> enclosing() const { return m_enclosing; }

private:
  std::unordered_map<std::string, std::any> m_values{};
  std::shared_ptr<Environment> m_enclosing = nullptr;
};

} // namespace seam::interpreter
