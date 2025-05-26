#include "pool.h"
#include <iostream>

namespace vk {
std::optional<CommandPool>
CommandPool::create(Device &device, vk::info::CommandPoolCreate info) {
  VkCommandPool commandPool;
  auto result = vkCreateCommandPool(*device, &info, nullptr, &commandPool);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  return CommandPool(commandPool, device);
}

std::optional<CommandBuffer> CommandPool::allocBuffer(bool secondary) const {
  VkCommandBuffer commandBuffer;

  vk::info::CommandBufferAllocate bufferInfo(commandPool, 1, secondary);

  if (vkAllocateCommandBuffers(device.raw(), &bufferInfo, &commandBuffer) !=
      VK_SUCCESS) {
    std::cerr << "Failed to allocate command buffer" << std::endl;
    return std::nullopt;
  }

  return commandBuffer;
}

std::optional<std::vector<CommandBuffer>>
CommandPool::allocBuffers(uint32_t count, bool secondary) const {
  std::vector<VkCommandBuffer> rawCommandBuffers;
  rawCommandBuffers.resize(count);

  vk::info::CommandBufferAllocate bufferInfo(commandPool, count, secondary);

  if (vkAllocateCommandBuffers(device.raw(), &bufferInfo,
                               rawCommandBuffers.data()) != VK_SUCCESS) {
    std::cerr << "Failed to allocate command buffers" << std::endl;
    return std::nullopt;
  }

  std::vector<CommandBuffer> commandBuffers;

  for (auto &rawCommandBuffer : rawCommandBuffers) {
    if (rawCommandBuffer == VK_NULL_HANDLE) {
      std::cerr << "Failed to allocate command buffer, got VK_NULL_HANDLE"
                << std::endl;
      return std::nullopt;
    }

    commandBuffers.emplace_back(rawCommandBuffer);
  }

  if (commandBuffers.size() != count) {
    std::cerr << "Failed to allocate all command buffers, allocated "
              << commandBuffers.size() << " buffers instead of " << count
              << std::endl;
    return std::nullopt;
  }

  return commandBuffers;
}
} // namespace vk
