module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;

import :info.sync.fenceCreate;

namespace vk {
std::optional<Fence> Fence::create(Device &device, bool makeSignalled) {
  vk::info::FenceCreate createInfo(makeSignalled);

  return create(device, createInfo);
}

std::optional<Fence> Fence::create(Device &device,
                                   vk::info::FenceCreate createInfo) {
  VkFence fence;

  auto result = vkCreateFence(*device, &createInfo, nullptr, &fence);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  Fence f(fence, device);

  return f;
}
} // namespace vk
