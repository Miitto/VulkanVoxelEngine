module;

#include "vulkan/vulkan_core.h"

export module vk:info.commands.poolCreate;

export namespace vk::info {
class CommandPoolCreate : public VkCommandPoolCreateInfo {

public:
  CommandPoolCreate(uint32_t queueFamily, bool resetable = false,
                    bool transient = false)
      : VkCommandPoolCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .queueFamilyIndex = queueFamily} {

    if (resetable) {
      flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    }

    if (transient) {
      flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    }
  }

  CommandPoolCreate &setFlags(VkCommandPoolCreateFlags f) {
    flags = f;
    return *this;
  }

  CommandPoolCreate &setQueueFamilyIndex(uint32_t index) {
    queueFamilyIndex = index;
    return *this;
  }

  CommandPoolCreate &setPNext(void *next) {
    pNext = next;
    return *this;
  }

  CommandPoolCreate &resetable() {
    flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    return *this;
  }

  CommandPoolCreate &transient() {
    flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    return *this;
  }
};
} // namespace vk::info
