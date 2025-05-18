#include "memory.h"

std::optional<DeviceMemory>
DeviceMemory::create(Device &device, VkMemoryAllocateInfo &info,
                     PhysicalDevice::MemoryProperties &memoryType) {
  VkDeviceMemory memory;
  if (vkAllocateMemory(*device, &info, nullptr, &memory) != VK_SUCCESS) {
    return std::nullopt;
  }

  return DeviceMemory(device, memory, info.allocationSize, memoryType);
}
