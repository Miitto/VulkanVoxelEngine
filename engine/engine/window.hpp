#pragma once

#include "engine/logger.hpp"
#include "engine/rect.hpp"
#include <GLFW/glfw3.h>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>

namespace engine {

class Window {
  std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window;
  std::optional<Dimensions> newSize = std::nullopt;

  static std::atomic_bool s_resizable;

  static void rawOnWindowResize(GLFWwindow *window, int width, int height);

  struct ResizeCallback {
    void *data = nullptr;
    std::function<void(engine::Dimensions, void *)> onResizeCallback;
  };

  std::optional<ResizeCallback> resizeCallback;

public:
  struct Attribs {
    const int32_t width;
    const int32_t height;
    const char *title;
    const bool resizable = true;
  };

  Window() = delete;

  Window(const Attribs &attribs)
      : window(nullptr, nullptr), resizeCallback(std::nullopt) {
    if (s_resizable != attribs.resizable) {
      glfwWindowHint(GLFW_RESIZABLE,
                     attribs.resizable ? GLFW_TRUE : GLFW_FALSE);
      s_resizable = attribs.resizable;
    }

    window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
        glfwCreateWindow(attribs.width, attribs.height, attribs.title, nullptr,
                         nullptr),
        glfwDestroyWindow);

    glfwSetWindowUserPointer(window.get(), this);
    glfwSetWindowSizeCallback(window.get(), rawOnWindowResize);
  }

  Window(const Window &) = delete;
  auto operator=(const Window &) -> Window & = delete;

  // Need to re-set the user pointer when moving the window
  Window(Window &&o) noexcept
      : window(std::move(o.window)),
        resizeCallback(std::move(o.resizeCallback)) {
    glfwSetWindowUserPointer(window.get(), this);
  }
  auto operator=(Window &&o) noexcept -> Window & {
    if (this != &o) {
      window = std::move(o.window);
      resizeCallback = std::move(o.resizeCallback);
      glfwSetWindowUserPointer(window.get(), this);
    }
    return *this;
  }

  void onWindowResize(Dimensions dim) {
    Logger::trace("Window resized to {}x{}. Callback: {}", dim.width,
                  dim.height, resizeCallback.has_value());
    newSize = dim;

    if (resizeCallback.has_value()) {
      resizeCallback->onResizeCallback(dim, resizeCallback->data);
    }
  }

  [[nodiscard]] auto getNewSize(const bool reset = true)
      -> std::optional<Dimensions> {
    if (newSize) {
      auto size = newSize;
      if (reset) {
        newSize = std::nullopt; // Reset after getting the size
      }
      return size;
    }
    return std::nullopt;
  }

  [[nodiscard]] auto get() const -> GLFWwindow * { return window.get(); }

  [[nodiscard]] auto isValid() const -> bool { return window != nullptr; }

  void
  setResizeCallback(void *data,
                    std::function<void(engine::Dimensions, void *)> callback) {
    resizeCallback =
        ResizeCallback{.data = data, .onResizeCallback = std::move(callback)};
  }
};
} // namespace engine
