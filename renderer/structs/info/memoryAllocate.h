#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
namespace info {
class MemoryAllocate : public VkMemoryAllocateInfo {

public:
  MemoryAllocate(uint32_t allocSize, uint32_t memoryTypeIndex)
      : VkMemoryAllocateInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                             .pNext = nullptr,
                             .allocationSize = allocSize,
                             .memoryTypeIndex = memoryTypeIndex} {}
};
} // namespace info
} // namespace vk
