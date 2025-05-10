#pragma once

#include "vulkan/vulkan.h"

class SemaphoreCreateInfoBuilder {
  VkSemaphoreCreateInfo createInfo;

public:
  SemaphoreCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
  }

  VkSemaphoreCreateInfo build() { return createInfo; }
};
