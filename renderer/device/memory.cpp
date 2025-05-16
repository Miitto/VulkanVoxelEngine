#include "memory.h"

std::optional<DeviceMemory> DeviceMemory::create(Device &device,
                                                 VkMemoryAllocateInfo &info) {
  VkDeviceMemory memory;
  if (vkAllocateMemory(*device, &info, nullptr, &memory) != VK_SUCCESS) {
    return std::nullopt;
  }
  auto memProperties = device.getPhysical()
                           .getMemoryProperties()
                           .memoryTypes[info.memoryTypeIndex]
                           .propertyFlags;
  bool isCoherent = (memProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
  return DeviceMemory(device, memory, info.allocationSize, isCoherent);
}
