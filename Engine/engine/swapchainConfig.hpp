#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

namespace engine {
auto chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats)
    -> vk::SurfaceFormatKHR {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
        availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

auto chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes)
    -> vk::PresentModeKHR {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
      return availablePresentMode;
    }
  }
  return vk::PresentModeKHR::eFifo;
}

auto chooseSwapExtent(GLFWwindow *window,
                      const vk::SurfaceCapabilitiesKHR &capabilities)
    -> vk::Extent2D {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {
      .width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width),
      .height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height)};
}

auto minImageCount(const vk::SurfaceCapabilitiesKHR &capabilities,
                   uint32_t desired) -> uint32_t {
  auto minImageCount = std::max(desired, capabilities.minImageCount);
  minImageCount = (capabilities.maxImageCount > 0 &&
                   minImageCount > capabilities.maxImageCount)
                      ? capabilities.maxImageCount
                      : minImageCount;

  return minImageCount;
}

auto desiredImageCount(const vk::SurfaceCapabilitiesKHR &capabilities)
    -> uint32_t {
  auto desired = capabilities.minImageCount + 1;

  if (capabilities.maxImageCount > 0 && desired > capabilities.maxImageCount) {
    desired = capabilities.maxImageCount;
  }

  return desired;
}

} // namespace engine
