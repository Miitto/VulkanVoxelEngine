#include "queue.h"

static std::unordered_map<VkPhysicalDevice,
                            std::vector<VkQueueFamilyProperties>>
      queueFamilies ();

std::vector<VkQueueFamilyProperties> Queue::all(PhysicalDevice &device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount,
                                           families.data());

  return families;
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
