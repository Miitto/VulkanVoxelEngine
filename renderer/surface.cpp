#include "surface.h"
#include <iostream>

std::optional<Surface> Surface::create(Instance &instance, Window &window) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(*instance, *window, nullptr, &surface) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create window surface." << std::endl;
    return std::nullopt;
  }
  return Surface(instance, surface);
}

Surface::~Surface() {
  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(*instance, surface, nullptr);
  }
}
