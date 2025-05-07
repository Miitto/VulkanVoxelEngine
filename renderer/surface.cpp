#include "surface.h"

#include <iostream>
#include <vulkan/vulkan.h>

std::optional<Surface> Surface::create(Instance &instance, Window &window) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(*instance, *window, nullptr, &surface) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create window surface." << std::endl;
    return std::nullopt;
  }
  return Surface(instance, surface);
}

SurfaceAttributes::SurfaceAttributes(PhysicalDevice &physicalDevice,
                                     Surface &surface) {
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physicalDevice, *surface,
                                            &capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface, &formatCount,
                                       nullptr);
  if (formatCount != 0) {
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice, *surface,
                                         &formatCount, formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(*physicalDevice, *surface,
                                            &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        *physicalDevice, *surface, &presentModeCount, presentModes.data());
  }
}
