#pragma once

#include "vulkan/vulkan.h"

class FenceCreateInfoBuilder {
  VkFenceCreateInfo createInfo;

public:
  FenceCreateInfoBuilder(bool signaled = false) {
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
  }

  VkFenceCreateInfo build() { return createInfo; }
};
