#include "logicalDevice.h"

std::optional<LogicalDevice>
LogicalDevice::create(PhysicalDevice &physicalDevice,
                      VkDeviceCreateInfo &createInfo) {
  VkDevice device;
  VkResult result =
      vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }
  return LogicalDevice(device);
}

std::optional<Queue> LogicalDevice::getQueue(uint32_t queueFamilyIndex,
                                             uint32_t queueIndex) {
  VkQueue queue;
  std::println("Getting queue for family index: {} and queue index: {}",
               queueFamilyIndex, queueIndex);
  std::cout << "Using logical device " << **device << std::endl;
  vkGetDeviceQueue(**device, queueFamilyIndex, queueIndex, &queue);
  if (queue == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  return Queue(queue);
}
