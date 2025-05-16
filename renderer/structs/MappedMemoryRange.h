#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
class MappedMemoryRange : public VkMappedMemoryRange {
public:
  MappedMemoryRange(VkDeviceMemory &memory, VkDeviceSize size,
                    VkDeviceSize offset = 0)
      : VkMappedMemoryRange{
            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .pNext = nullptr,
            .memory = memory,
            .offset = offset,
            .size = size,
        } {}
};
} // namespace vk
