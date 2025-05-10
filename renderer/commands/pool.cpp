#include "pool.h"
#include "vkStructs/commands/bufferAllocateInfo.h"

std::optional<CommandPool> CommandPool::create(Device &device,
                                               VkCommandPoolCreateInfo info) {
  VkCommandPool commandPool;
  auto result = vkCreateCommandPool(*device, &info, nullptr, &commandPool);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  return CommandPool(commandPool, device);
}

std::optional<CommandBuffer> CommandPool::allocBuffer(bool secondary) {
  VkCommandBuffer commandBuffer;

  CommandBufferAllocateInfoBuilder allocInfo(**this, secondary);

  auto bufferInfo = allocInfo.build();

  if (vkAllocateCommandBuffers(device.raw(), &bufferInfo, &commandBuffer) !=
      VK_SUCCESS) {
    std::cerr << "Failed to allocate command buffer" << std::endl;
    return std::nullopt;
  }

  return std::move(commandBuffer);
}

std::optional<std::vector<CommandBuffer>>
CommandPool::allocBuffers(uint32_t count, bool secondary) {
  std::vector<VkCommandBuffer> rawCommandBuffers;
  rawCommandBuffers.reserve(count);

  CommandBufferAllocateInfoBuilder allocInfo(**this, secondary);
  allocInfo.setCount(count);

  auto bufferInfo = allocInfo.build();

  if (vkAllocateCommandBuffers(device.raw(), &bufferInfo,
                               rawCommandBuffers.data()) != VK_SUCCESS) {
    std::cerr << "Failed to allocate command buffers" << std::endl;
    return std::nullopt;
  }

  std::vector<CommandBuffer> commandBuffers;

  for (auto &rawCommandBuffer : rawCommandBuffers) {
    commandBuffers.emplace_back(rawCommandBuffer);
  }

  return std::move(commandBuffers);
}
