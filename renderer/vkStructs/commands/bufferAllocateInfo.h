#pragma once

#include "vulkan/vulkan.h"

class CommandBufferAllocateInfoBuilder {
  VkCommandBufferAllocateInfo createInfo;

public:
  CommandBufferAllocateInfoBuilder(VkCommandPool commandPool,
                                   bool secondary = false) {
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.commandPool = commandPool;
    createInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY
                                 : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    createInfo.commandBufferCount = 1;
  }

  CommandBufferAllocateInfoBuilder &setCount(uint32_t count) {
    createInfo.commandBufferCount = count;
    return *this;
  }

  VkCommandBufferAllocateInfo build() { return createInfo; }
};
