#pragma once

#include <GLFW/glfw3.h>
#include <optional>
#include <print>

class Window {
  GLFWwindow *window = nullptr;

  Window(const char *name, const uint32_t width, const uint32_t height);

public:
  // Move only, and clean up old window on move
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  Window(Window &&o) noexcept : window(o.window) { o.window = nullptr; }
  Window &operator=(Window &&o) noexcept {
    window = o.window;
    o.window = nullptr;
    return *this;
  };
  ~Window() {
    if (window != nullptr) {
      std::println("Destroying window");
      glfwDestroyWindow(window);
    }
  }
  static std::optional<Window> create(const char *name, const uint32_t width,
                                      const uint32_t height);
  bool shouldClose() { return glfwWindowShouldClose(window); }

  GLFWwindow *operator*() { return window; }
};
