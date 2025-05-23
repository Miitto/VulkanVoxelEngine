#pragma once

#include "commands/buffer.h"
#include "device/device.h"
#include "structs/info/commands/bufferAllocate.h"
#include "structs/info/commands/commandPoolCreate.h"
#include "vulkan/vulkan_core.h"
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
  operator VkCommandPool() { return commandPool; }

  const Device::Ref &getDevice() const { return device; }

  static std::optional<CommandPool> create(Device &device,
                                           vk::info::CommandPoolCreate info);

  std::optional<CommandBuffer> allocBuffer(bool secondary = false) const;
  std::optional<std::vector<CommandBuffer>>
  allocBuffers(uint32_t count, bool secondary = false) const;
  std::optional<std::vector<CommandBuffer>>
  allocBuffers(vk::info::CommandBufferAllocate &info) const;
};
