module;

#include <GLFW/glfw3.h>
#include <algorithm>
#include <limits>
#include <optional>
#include <vulkan/vulkan_core.h>

module vk;

vk::Window::Window(const char *name, const uint32_t width,
                   const uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  this->window =
      glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), name,
                       nullptr, nullptr);
}

std::optional<vk::Window> vk::Window::create(const char *name,
                                             const uint32_t width,
                                             const uint32_t height) {
  Window window(name, width, height);

  if (window.window == nullptr) {
    return std::nullopt;
  }

  return window;
}

VkExtent2D vk::Window::getExtent() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {.width = static_cast<uint32_t>(width),
          .height = static_cast<uint32_t>(height)};
}

VkExtent2D vk::Window::getExtent(VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  VkExtent2D actualExtent = {.width = static_cast<uint32_t>(width),
                             .height = static_cast<uint32_t>(height)};

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);

  return actualExtent;
}
