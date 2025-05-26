#pragma once

#include "device/device.h"
#include "vulkan/vulkan.h"
#include <optional>

namespace vk {
class Semaphore {
  VkSemaphore semaphore;
  Device::Ref device;

  Semaphore() = delete;
  Semaphore(const Semaphore &) = delete;
  Semaphore &operator=(const Semaphore &) = delete;

public:
  Semaphore(VkSemaphore semaphore, Device &device)
      : semaphore(semaphore), device(device.ref()) {}

  Semaphore(Semaphore &&other) noexcept
      : semaphore(other.semaphore), device(other.device) {
    other.semaphore = VK_NULL_HANDLE;
  }

  Semaphore operator=(Semaphore &&other) noexcept {
    if (this != &other) {
      semaphore = other.semaphore;
      device = other.device;
      other.semaphore = VK_NULL_HANDLE;
    }
    return std::move(*this);
  }

  static std::optional<Semaphore> create(Device &device);
  static std::optional<Semaphore> create(Device &device,
                                         VkSemaphoreCreateInfo info);

  VkSemaphore operator*() { return semaphore; }
  operator VkSemaphore() { return semaphore; }

  ~Semaphore() {
    if (semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(**device, semaphore, nullptr);
      semaphore = VK_NULL_HANDLE;
    }
  }
};
} // namespace vk
