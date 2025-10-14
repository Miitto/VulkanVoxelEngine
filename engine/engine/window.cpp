#include "window.hpp"

namespace {
void rawOnWindowResize(GLFWwindow *window, int width, int height) noexcept {
  auto *self = static_cast<engine::Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->onWindowResize({.width = width, .height = height});
  }
}

void rawOnWindowKeypress(GLFWwindow *window, int key, int scancode, int action,
                         int mods) noexcept {
  auto *self = static_cast<engine::Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->onWindowKeypress(key, scancode, action, mods);
  }
}

void rawOnWindowMouseMove(GLFWwindow *window, double xpos,
                          double ypos) noexcept {
  auto *self = static_cast<engine::Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->onWindowMouseMove(xpos, ypos);
  }
}

void rawOnWindowMouseButton(GLFWwindow *window, int button, int action,
                            int mods) noexcept {
  auto *self = static_cast<engine::Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->onWindowMouseButton(button, action, mods);
  }
}

} // namespace

namespace engine {
std::atomic_bool Window::s_resizable = true;

#pragma region Callbacks
void Window::onWindowResize(Dimensions dim) noexcept {
  newSize = dim;
  if (resizeCallback.has_value()) {
    (*resizeCallback)(dim);
  }
}

void Window::onWindowKeypress(int key, int scancode, int action,
                              int mods) noexcept {
  if (keyCallback.has_value()) {
    auto a = static_cast<KeyAction>(action);

    auto k = static_cast<Key>(key);

    (*keyCallback)(k, scancode, a, mods);
  }
}

void Window::onWindowMouseMove(double xpos, double ypos) noexcept {
  if (mouseMoveCallback.has_value()) {
    (*mouseMoveCallback)(xpos, ypos);
  }
}

void Window::onWindowMouseButton(int button, int action, int mods) noexcept {
  if (mouseButtonCallback.has_value()) {
    (*mouseButtonCallback)(button, action, mods);
  }
}

#pragma endregion

Window::Window(const Attribs &attribs) noexcept
    : window(nullptr, nullptr), resizeCallback(std::nullopt) {
  if (s_resizable != attribs.resizable) {
    glfwWindowHint(GLFW_RESIZABLE, attribs.resizable ? GLFW_TRUE : GLFW_FALSE);
    s_resizable = attribs.resizable;
  }

  window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      glfwCreateWindow(attribs.width, attribs.height, attribs.title, nullptr,
                       nullptr),
      glfwDestroyWindow);

  glfwSetWindowUserPointer(window.get(), this);
  glfwSetWindowSizeCallback(window.get(), rawOnWindowResize);
  glfwSetKeyCallback(window.get(), rawOnWindowKeypress);
  glfwSetCursorPosCallback(window.get(), rawOnWindowMouseMove);
  glfwSetMouseButtonCallback(window.get(), rawOnWindowMouseButton);
}

Window::Window(Window &&o) noexcept
    : window(std::move(o.window)), resizeCallback(std::move(o.resizeCallback)) {
  glfwSetWindowUserPointer(window.get(), this);
}

auto Window::operator=(Window &&o) noexcept -> Window & {
  if (this != &o) {
    window = std::move(o.window);
    resizeCallback = std::move(o.resizeCallback);
    glfwSetWindowUserPointer(window.get(), this);
  }
  return *this;
}

void Window::close() noexcept {
  glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
}

[[nodiscard]] auto Window::getNewSize(const bool reset) noexcept
    -> std::optional<Dimensions> {
  if (newSize) {
    auto size = newSize;
    if (reset) {
      newSize = std::nullopt;
    }
    return size;
  }
  return std::nullopt;
}

void Window::setCursorMode(CursorMode mode) noexcept {
  switch (mode) {
  case CursorMode::Normal:
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    break;
  case CursorMode::Hidden:
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    break;
  case CursorMode::Disabled:
    glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    break;
  }

  m_cursorMode = mode;
}

void Window::setResizeCallback(ResizeCallback callback) noexcept {
  resizeCallback = std::move(callback);
}

void Window::setKeyCallback(KeyCallback callback) noexcept {
  keyCallback = std::move(callback);
}

void Window::setMouseMoveCallback(MouseMoveCallback callback) noexcept {
  mouseMoveCallback = std::move(callback);
}

void Window::setMouseButtonCallback(MouseButtonCallback callback) noexcept {
  mouseButtonCallback = std::move(callback);
}

} // namespace engine
