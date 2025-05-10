#pragma once

#include "vulkan/vulkan.h"

class CommandBufferBeginInfoBuilder {
  VkCommandBufferBeginInfo createInfo;

public:
  CommandBufferBeginInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pInheritanceInfo = nullptr;
  }

  CommandBufferBeginInfoBuilder &setFlags(VkCommandBufferUsageFlags flags) {
    createInfo.flags = flags;
    return *this;
  }

  CommandBufferBeginInfoBuilder &setPNext(void *pNext) {
    createInfo.pNext = pNext;
    return *this;
  }

  CommandBufferBeginInfoBuilder &
  setInheritanceInfo(VkCommandBufferInheritanceInfo &inheritanceInfo) {
    createInfo.pInheritanceInfo = &inheritanceInfo;
    return *this;
  }

  CommandBufferBeginInfoBuilder &oneTime() {
    createInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    return *this;
  }

  CommandBufferBeginInfoBuilder &renderPassContinue() {
    createInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    return *this;
  }

  CommandBufferBeginInfoBuilder &simultaneousUse() {
    createInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    return *this;
  }

  VkCommandBufferBeginInfo build() { return createInfo; }
};
