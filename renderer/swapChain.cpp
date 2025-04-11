#include "swapChain.h"

SwapChainSupport::SwapChainSupport(PhysicalDevice &physicalDevice,
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
