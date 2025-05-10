#pragma once

#include "commands/buffer.h"
#include "device/device.h"
#include "vulkan/vulkan.h"
#include <cstdint>
#include <optional>

class CommandPool {
  VkCommandPool commandPool;
  Device::Ref device;

  CommandPool() = delete;
  CommandPool(const CommandPool &) = delete;
  CommandPool &operator=(const CommandPool &) = delete;
  CommandPool &operator=(CommandPool &&) = delete;

public:
  CommandPool(VkCommandPool commandPool, Device &device)
      : commandPool(commandPool), device(device.ref()) {}
  ~CommandPool() {
    if (commandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(**device, commandPool, nullptr);
    }
  }

  CommandPool(CommandPool &&other) noexcept
      : commandPool(other.commandPool), device(other.device) {
    other.commandPool = VK_NULL_HANDLE;
  }

  VkCommandPool operator*() { return commandPool; }

  static std::optional<CommandPool> create(Device &device,
                                           VkCommandPoolCreateInfo info);

  std::optional<CommandBuffer> allocBuffer(bool secondary = false);
  std::optional<std::vector<CommandBuffer>>
  allocBuffers(uint32_t count, bool secondary = false);
};
