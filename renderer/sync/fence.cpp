#include "fence.h"
#include "vkStructs/sync/fenceCreateInfo.h"

std::optional<Fence> Fence::create(Device &device, bool makeSignalled) {
  FenceCreateInfoBuilder createInfo(makeSignalled);

  return create(device, createInfo.build());
}

std::optional<Fence> Fence::create(Device &device,
                                   VkFenceCreateInfo createInfo) {
  VkFence fence;

  auto result = vkCreateFence(*device, &createInfo, nullptr, &fence);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  Fence f(fence, device);

  return std::move(f);
}
