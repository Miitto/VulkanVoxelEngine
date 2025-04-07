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

LogicalDevice::~LogicalDevice() {
  if (device != nullptr) {
    vkDestroyDevice(device, nullptr);
  }
}
