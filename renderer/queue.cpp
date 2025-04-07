#include "queue.h"

std::vector<VkQueueFamilyProperties> Queue::all(PhysicalDevice &device) {
  if (queueFamilies.contains(*device)) {
    return queueFamilies.at(*device);
  }

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount,
                                           families.data());

  queueFamilies[*device] = families;
  return queueFamilies.at(*device);
}

bool Queue::canPresent(PhysicalDevice &device,
                       VkQueueFamilyProperties &queueFamily,
                       VkSurfaceKHR surface) {
  VkBool32 presentSupport = false;
  vkGetPhysicalDeviceSurfaceSupportKHR(*device, queueFamily.queueCount, surface,
                                       &presentSupport);
  return presentSupport;
}

bool Queue::hasGraphics(VkQueueFamilyProperties &queueFamily) {
  return queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}
