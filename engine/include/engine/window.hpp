#pragma once

#include "engine/rect.hpp"
#include <GLFW/glfw3.h>
#include <atomic>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <optional>

namespace engine {

enum class KeyAction {
  Press = GLFW_PRESS,
  Release = GLFW_RELEASE,
  Repeat = GLFW_REPEAT
};

enum class Key {
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  Esc = GLFW_KEY_ESCAPE,
  Enter = GLFW_KEY_ENTER,
  One = GLFW_KEY_1,
  Two = GLFW_KEY_2,
  Three = GLFW_KEY_3,
  Four = GLFW_KEY_4,
  Five = GLFW_KEY_5,
  Six = GLFW_KEY_6,
  Seven = GLFW_KEY_7,
  Eight = GLFW_KEY_8,
  Nine = GLFW_KEY_9,
  Zero = GLFW_KEY_0,
  Space = GLFW_KEY_SPACE,
  Ctrl = GLFW_KEY_LEFT_CONTROL,
};

enum class CursorMode {
  Normal = GLFW_CURSOR_NORMAL,
  Hidden = GLFW_CURSOR_HIDDEN,
  Disabled = GLFW_CURSOR_DISABLED
};

class Window {
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

  [[nodiscard]] inline glm::ivec2 getFramebufferSize() const noexcept {
    int width, height;
    glfwGetFramebufferSize(window.get(), &width, &height);
    return {width, height};
  }

  [[nodiscard]] inline auto get() const noexcept -> GLFWwindow * {
    return window.get();
  }

  [[nodiscard]] inline auto isValid() const noexcept -> bool {
    return window != nullptr;
  }

  void close() noexcept;
  void setCursorMode(CursorMode mode) noexcept;
  [[nodiscard]] inline CursorMode getCursorMode() const noexcept {
    return m_cursorMode;
  }

  [[nodiscard]] inline auto shouldClose() const noexcept -> bool {
    return glfwWindowShouldClose(window.get()) == GLFW_TRUE;
  }

  using ResizeCallback = std::function<void(engine::Dimensions)>;
  void setResizeCallback(ResizeCallback callback) noexcept;

  using KeyCallback = std::function<void(Key, int, KeyAction, int)>;
  void setKeyCallback(KeyCallback callback) noexcept;

  using MouseMoveCallback = std::function<void(double, double)>;
  void setMouseMoveCallback(MouseMoveCallback callback) noexcept;

  using MouseButtonCallback = std::function<void(int, int, int)>;
  void setMouseButtonCallback(MouseButtonCallback callback) noexcept;

protected:
  std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window;
  std::optional<Dimensions> newSize = std::nullopt;

  static std::atomic_bool s_resizable;

  std::optional<ResizeCallback> resizeCallback;

  std::optional<KeyCallback> keyCallback;

  std::optional<MouseMoveCallback> mouseMoveCallback;

  std::optional<MouseButtonCallback> mouseButtonCallback;

  CursorMode m_cursorMode = CursorMode::Normal;
};
} // namespace engine
