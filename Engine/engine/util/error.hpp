#pragma once

#include <string>
#include <vulkan/vulkan_raii.hpp>

// TODO: Make a formatter for Result
class Result {
  vk::Result res;

public:
  Result(vk::Result res) : res(res) {}

  operator std::string() const {
    switch (res) {
    case vk::Result::eSuccess: {
      return "Success";
    }
    default: {
      return "Error: " + std::to_string(static_cast<int>(res));
    }
    }
  }

  [[nodiscard]] auto toString() const -> std::string {
    return static_cast<std::string>(*this);
  }
};

auto format_as(Result f) { return f.toString(); }
