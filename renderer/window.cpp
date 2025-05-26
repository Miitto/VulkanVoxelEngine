#include "window.h"
#include <algorithm>
#include <vulkan/vulkan_core.h>

namespace vk {
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

VkExtent2D Window::getExtent() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {.width = static_cast<uint32_t>(width),
          .height = static_cast<uint32_t>(height)};
}

VkExtent2D Window::getExtent(VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
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
}
} // namespace vk
