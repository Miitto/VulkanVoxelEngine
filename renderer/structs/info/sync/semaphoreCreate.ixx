module;

#include "vulkan/vulkan_core.h"

export module vk:info.sync.semaphoreCreate;

export namespace vk {
namespace info {
class SemaphoreCreate : public VkSemaphoreCreateInfo {

public:
  SemaphoreCreate()
      : VkSemaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                              .pNext = nullptr,
                              .flags = 0} {}
};
} // namespace info
} // namespace vk
