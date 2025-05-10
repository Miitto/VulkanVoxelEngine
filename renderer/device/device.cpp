#include "device.h"

std::optional<Device> Device::create(PhysicalDevice &physicalDevice,
                                     VkDeviceCreateInfo &createInfo) {
  VkDevice device;
  VkResult result =
      vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }
  return Device(device);
}

std::optional<Queue> Device::getQueue(uint32_t queueFamilyIndex,
                                      uint32_t queueIndex) {
  VkQueue queue;
  vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
  if (queue == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  return Queue(queue);
}
