#pragma once

#include "vulkan/vulkan.h"

class CommandPoolCreateInfoBuilder {
  VkCommandPoolCreateInfo createInfo;

public:
  CommandPoolCreateInfoBuilder(uint32_t queueFamilyIndex) {
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueFamilyIndex = queueFamilyIndex;
  }

  CommandPoolCreateInfoBuilder &setFlags(VkCommandPoolCreateFlags flags) {
    createInfo.flags = flags;
    return *this;
  }

  CommandPoolCreateInfoBuilder &setQueueFamilyIndex(uint32_t index) {
    createInfo.queueFamilyIndex = index;
    return *this;
  }

  CommandPoolCreateInfoBuilder &setPNext(void *pNext) {
    createInfo.pNext = pNext;
    return *this;
  }

  CommandPoolCreateInfoBuilder &resetable() {
    createInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    return *this;
  }

  CommandPoolCreateInfoBuilder &transient() {
    createInfo.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    return *this;
  }

  VkCommandPoolCreateInfo build() { return createInfo; }
};
