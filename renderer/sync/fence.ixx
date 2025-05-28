module;

#include "vulkan/vulkan_core.h"
#include <optional>

export module vk:fence;

import :device;
import :info.sync.fenceCreate;

export namespace vk {
class Fence {
  VkFence fence;
  Device::Ref device;

  Fence() = delete;
  Fence(const Fence &) = delete;
  Fence &operator=(const Fence &) = delete;

public:
  Fence(VkFence semaphore, Device &device)
      : fence(semaphore), device(device.ref()) {}

  Fence(Fence &&other) noexcept : fence(other.fence), device(other.device) {
    other.fence = VK_NULL_HANDLE;
  }

  Fence operator=(Fence &&other) noexcept {
    if (this != &other) {
      fence = other.fence;
      device = other.device;
      other.fence = VK_NULL_HANDLE;
    }
    return std::move(*this);
  }

  static std::optional<Fence> create(Device &device,
                                     bool makeSignalled = false);
  static std::optional<Fence> create(Device &device,
                                     vk::info::FenceCreate info);

  void wait(bool autoReset = false, uint64_t timeout = UINT64_MAX) {
    vkWaitForFences(**device, 1, &fence, VK_TRUE, timeout);

    if (autoReset) {
      reset();
    }
  }

  void reset() { vkResetFences(**device, 1, &fence); }

  VkFence operator*() { return fence; }
  operator VkFence() { return fence; }

  ~Fence() {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(**device, fence, nullptr);
      fence = VK_NULL_HANDLE;
    }
  }
};
} // namespace vk
