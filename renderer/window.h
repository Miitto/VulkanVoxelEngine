#pragma once

#include <GLFW/glfw3.h>
#include <optional>
#include <print>

class WindowPtr {
  GLFWwindow *window = nullptr;

public:
  WindowPtr(GLFWwindow *window) : window(window) {}
  ~WindowPtr() {
    if (window != nullptr) {
      std::println("Destroying window");
      glfwDestroyWindow(window);
    }
  }

  WindowPtr(const WindowPtr &) = delete;
  WindowPtr &operator=(const WindowPtr &) = delete;
  WindowPtr(WindowPtr &&o) noexcept : window(std::move(o.window)) {
    o.window = nullptr;
  }
  WindowPtr &operator=(WindowPtr &&o) noexcept {
    window = std::move(o.window);
    o.window = nullptr;
    return *this;
  }

  GLFWwindow *operator*() { return window; }
};

class Window {
  GLFWwindow *window = nullptr;

  Window(const char *name, const uint32_t width, const uint32_t height);

public:
  static std::optional<Window> create(const char *name, const uint32_t width,
                                      const uint32_t height);
  bool shouldClose() { return glfwWindowShouldClose(window); }

  GLFWwindow *operator*() { return window; }
};
