#pragma once

#include "device/device.h"
#include "vulkan/vulkan.h"
#include <optional>

class Fence {
  VkFence fence;
  Device::Ref device;

  Fence() = delete;
  Fence(const Fence &) = delete;
  Fence &operator=(const Fence &) = delete;
  Fence &operator=(Fence &&) = delete;

public:
  Fence(VkFence semaphore, Device &device)
      : fence(semaphore), device(device.ref()) {}

  Fence(Fence &&other) noexcept : fence(other.fence), device(other.device) {
    other.fence = VK_NULL_HANDLE;
  }

  static std::optional<Fence> create(Device &device,
                                     bool makeSignalled = false);
  static std::optional<Fence> create(Device &device, VkFenceCreateInfo info);

  void wait(bool autoReset = false, uint64_t timeout = UINT64_MAX) {
    vkWaitForFences(**device, 1, &fence, VK_TRUE, timeout);

    if (autoReset) {
      reset();
    }
  }

  void reset() { vkResetFences(**device, 1, &fence); }

  VkFence operator*() { return fence; }

  ~Fence() {
    if (fence != VK_NULL_HANDLE) {
      vkDestroyFence(**device, fence, nullptr);
      fence = VK_NULL_HANDLE;
    }
  }
};
