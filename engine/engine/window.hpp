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

  struct ResizeCallback {
    void *data = nullptr;
    std::function<void(engine::Dimensions, void *)> onResizeCallback;
  };

  std::optional<ResizeCallback> resizeCallback;

  struct KeyCallback {
    void *data = nullptr;
    std::function<void(int, int, int, int, void *)> onKeyCallback;
  };

  std::optional<KeyCallback> keyCallback;

  struct MouseMoveCallback {
    void *data = nullptr;
    std::function<void(double, double, void *)> onMouseMoveCallback;
  };

  std::optional<MouseMoveCallback> mouseMoveCallback;

  struct MouseButtonCallback {
    void *data = nullptr;
    std::function<void(int, int, int, void *)> onMouseButtonCallback;
  };

  std::optional<MouseButtonCallback> mouseButtonCallback;

public:
  struct Attribs {
    const int32_t width;
    const int32_t height;
    const char *title;
    const bool resizable = true;
  };

  Window() = delete;

  Window(const Attribs &attribs) noexcept;

  Window(const Window &) = delete;
  auto operator=(const Window &) -> Window & = delete;

  // Need to re-set the user pointer when moving the window
  Window(Window &&o) noexcept;
  auto operator=(Window &&o) noexcept -> Window &;

  void onWindowResize(Dimensions dim) noexcept;
  void onWindowKeypress(int key, int scancode, int action, int mods) noexcept;
  void onWindowMouseMove(double xpos, double ypos) noexcept;
  void onWindowMouseButton(int button, int action, int mods) noexcept;

  [[nodiscard]] auto getNewSize(const bool reset = true) noexcept
      -> std::optional<Dimensions>;

  [[nodiscard]] inline auto get() const noexcept -> GLFWwindow * {
    return window.get();
  }

  [[nodiscard]] inline auto isValid() const noexcept -> bool {
    return window != nullptr;
  }

  void close() noexcept;

  void setResizeCallback(
      void *data,
      std::function<void(engine::Dimensions, void *)> callback) noexcept;

  void setKeyCallback(
      void *data,
      std::function<void(int, int, int, int, void *)> callback) noexcept;

  void setMouseMoveCallback(
      void *data,
      std::function<void(double, double, void *)> callback) noexcept;

  void setMouseButtonCallback(
      void *data, std::function<void(int, int, int, void *)> callback) noexcept;
};
} // namespace engine
