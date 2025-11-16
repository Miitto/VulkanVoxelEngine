#include "engine/window.hpp"

namespace engine {
std::atomic_bool Window::s_resizable = true;

#pragma region Callbacks

Window::Window(const Attribs &attribs) noexcept : window(nullptr, nullptr) {
  if (s_resizable != attribs.resizable) {
    glfwWindowHint(GLFW_RESIZABLE, attribs.resizable ? GLFW_TRUE : GLFW_FALSE);
    s_resizable = attribs.resizable;
  }

  window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      glfwCreateWindow(attribs.width, attribs.height, attribs.title, nullptr,
                       nullptr),
      glfwDestroyWindow);
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

void Window::setUserPtr(void *ptr) {
  glfwSetWindowUserPointer(window.get(), ptr);
}
void *Window::getUserPtr(GLFWwindow *window) {
  return glfwGetWindowUserPointer(window);
}
} // namespace engine
