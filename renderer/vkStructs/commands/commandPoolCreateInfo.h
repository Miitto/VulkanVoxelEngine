#pragma once

#include "vulkan/vulkan_core.h"

class CommandPoolCreateInfo : public VkCommandPoolCreateInfo {

public:
  CommandPoolCreateInfo(uint32_t queueFamily, bool resetable = false,
                        bool transient = false) {
    sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pNext = nullptr;
    flags = 0;
    queueFamilyIndex = queueFamily;

    if (resetable) {
      flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    if (transient) {
      flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }
  }

  CommandPoolCreateInfo &setFlags(VkCommandPoolCreateFlags flags) {
    this->flags = flags;
    return *this;
  }

  CommandPoolCreateInfo &setQueueFamilyIndex(uint32_t index) {
    queueFamilyIndex = index;
    return *this;
  }

  CommandPoolCreateInfo &setPNext(void *next) {
    pNext = next;
    return *this;
  }

  CommandPoolCreateInfo &resetable() {
    flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    return *this;
  }

  CommandPoolCreateInfo &transient() {
    flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    return *this;
  }
};
