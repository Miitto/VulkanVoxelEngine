#include "window.hpp"

namespace engine {
std::atomic_bool Window::s_resizable = true;

void Window::rawOnWindowResize(GLFWwindow *window, int width, int height) {
  auto *self = static_cast<Window *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->onWindowResize({.width = width, .height = height});
  }
}
} // namespace engine
