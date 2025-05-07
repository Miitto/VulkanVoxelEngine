#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <optional>
#include <print>

class Window {
  GLFWwindow *window = nullptr;

public:
  Window(GLFWwindow *window) : window(window) {}
  Window(const char *name, const uint32_t width, const uint32_t height);
  ~Window() {
    if (window != nullptr) {
      std::println("Destroying window");
      glfwDestroyWindow(window);
    }
  }

  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&o) noexcept : window(std::move(o.window)) {
    o.window = nullptr;
  }
  Window &operator=(Window &&o) noexcept {
    window = std::move(o.window);
    o.window = nullptr;
    return *this;
  }

  GLFWwindow *operator*() { return window; }

public:
  static std::optional<Window> create(const char *name, const uint32_t width,
                                      const uint32_t height);
  bool shouldClose() { return glfwWindowShouldClose(window); }
};
