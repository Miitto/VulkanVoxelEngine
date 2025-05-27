module;

#include "vulkan/vulkan_core.h"

export module vk:info.commands.bufferAllocate;

export namespace vk {
namespace info {
class CommandBufferAllocate : public VkCommandBufferAllocateInfo {

public:
  CommandBufferAllocate(VkCommandPool commandPool, uint32_t count = 1,
                        bool secondary = false)
      : VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY
                               : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count} {}
};
} // namespace info
} // namespace vk
