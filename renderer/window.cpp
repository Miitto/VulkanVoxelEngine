#include "window.h"

Window::Window(const char *name, const uint32_t width, const uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  this->window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

std::optional<Window> Window::create(const char *name, const uint32_t width,
                                     const uint32_t height) {
  Window window(name, width, height);

  if (window.window == nullptr) {
    return std::nullopt;
  }

  return std::move(window);
}
