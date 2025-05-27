module;

#include "vulkan/vulkan_core.h"

export module vk:info.sync.fenceCreate;

export namespace vk {
namespace info {
class FenceCreate : public VkFenceCreateInfo {
public:
  FenceCreate(bool signaled = false)
      : VkFenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                          .pNext = nullptr,
                          .flags = signaled ? static_cast<uint32_t>(
                                                  VK_FENCE_CREATE_SIGNALED_BIT)
                                            : 0u} {}
};
} // namespace info
} // namespace vk
