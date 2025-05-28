module;

#include "vulkan/vulkan_core.h"

export module vk:info.commands.bufferBegin;

export namespace vk::info {

class CommandBufferBegin : public VkCommandBufferBeginInfo {

public:
  CommandBufferBegin()
      : VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr} {}

  CommandBufferBegin &oneTime() {
    flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return *this;
  }

  CommandBufferBegin &renderPassContinue() {
    flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    return *this;
  }

  CommandBufferBegin &simultaneousUse() {
    flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    return *this;
  }
};
} // namespace vk::info
