module;

#include "vulkan/vulkan_core.h"

export module vk:info.memoryAllocate;

export namespace vk {
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
