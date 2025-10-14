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
    resizeCallback->onResizeCallback(dim, resizeCallback->data);
  }
}

void Window::onWindowKeypress(int key, int scancode, int action,
                              int mods) noexcept {
  if (keyCallback.has_value()) {
    keyCallback->onKeyCallback(key, scancode, action, mods, keyCallback->data);
  }
}

void Window::onWindowMouseMove(double xpos, double ypos) noexcept {
  if (mouseMoveCallback.has_value()) {
    mouseMoveCallback->onMouseMoveCallback(xpos, ypos, mouseMoveCallback->data);
  }
}

void Window::onWindowMouseButton(int button, int action, int mods) noexcept {
  if (mouseButtonCallback.has_value()) {
    mouseButtonCallback->onMouseButtonCallback(button, action, mods,
                                               mouseButtonCallback->data);
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
      newSize = std::nullopt; // Reset after getting the size
    }
    return size;
  }
  return std::nullopt;
}

void Window::setResizeCallback(
    void *data,
    std::function<void(engine::Dimensions, void *)> callback) noexcept {
  resizeCallback =
      ResizeCallback{.data = data, .onResizeCallback = std::move(callback)};
}

void Window::setKeyCallback(
    void *data,
    std::function<void(int, int, int, int, void *)> callback) noexcept {
  keyCallback = KeyCallback{.data = data, .onKeyCallback = std::move(callback)};
}

void Window::setMouseMoveCallback(
    void *data, std::function<void(double, double, void *)> callback) noexcept {
  mouseMoveCallback = MouseMoveCallback{
      .data = data, .onMouseMoveCallback = std::move(callback)};
}

void Window::setMouseButtonCallback(
    void *data, std::function<void(int, int, int, void *)> callback) noexcept {
  mouseButtonCallback = MouseButtonCallback{
      .data = data, .onMouseButtonCallback = std::move(callback)};
}

} // namespace engine
