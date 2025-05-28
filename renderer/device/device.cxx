module;

#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

import :info.memoryAllocate;

namespace vk {
std::optional<Device>
Device::create(PhysicalDevice &physicalDevice,
               vk::info::DeviceCreate &createInfo) noexcept {

  VkDevice device;
  VkResult result =
      vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }
  return Device(device, physicalDevice);
}

std::optional<Queue> Device::getQueue(uint32_t queueFamilyIndex,
                                      uint32_t queueIndex) {
  VkQueue queue;
  vkGetDeviceQueue(m_device, queueFamilyIndex, queueIndex, &queue);
  if (queue == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  return Queue(queue, queueFamilyIndex);
}

std::optional<Swapchain>
Device::createSwapchain(vk::info::SwapchainCreate &info) {
  return Swapchain::create(*this, info);
}

std::optional<CommandPool>
Device::createCommandPool(vk::info::CommandPoolCreate &info) {
  return CommandPool::create(*this, info);
}

std::optional<Semaphore> Device::createSemaphore() {
  return Semaphore::create(*this);
}

std::optional<Fence> Device::createFence(bool signaled) {
  return Fence::create(*this, signaled);
}

std::optional<Buffer> Device::createBuffer(vk::info::BufferCreate &info) {
  return Buffer::create(*this, info);
}

std::optional<VertexBuffer>
Device::createVertexBuffer(vk::info::VertexBufferCreate &info) {
  return VertexBuffer::create(*this, info);
}

std::optional<IndexBuffer>
Device::createIndexBuffer(vk::info::IndexBufferCreate &info) {
  return IndexBuffer::create(*this, info);
}

std::optional<UniformBuffer>
Device::createUniformBuffer(vk::info::UniformBufferCreate &info) {
  return UniformBuffer::create(*this, info);
}

std::optional<DeviceMemory>
Device::allocateMemory(Buffer &buffer, VkMemoryPropertyFlags properties) {
  auto memoryReqs = buffer.getMemoryRequirements();

  return allocateMemory(memoryReqs, properties);
}

std::optional<DeviceMemory>
Device::allocateMemory(std::span<Buffer *> buffers,
                       VkMemoryPropertyFlags properties) {
  if (buffers.empty()) {
    return std::nullopt;
  }

  if (buffers.size() == 1) {
    return allocateMemory(*buffers[0], properties);
  }

  auto memoryReqs = buffers[0]->getMemoryRequirements();
  for (size_t i = 1; i < buffers.size(); i++) {
    VkMemoryRequirements reqs = buffers[i]->getMemoryRequirements();
    memoryReqs.size += reqs.size;
    memoryReqs.memoryTypeBits &= reqs.memoryTypeBits;
  }

  return allocateMemory(memoryReqs, properties);
}

std::optional<DeviceMemory>
Device::allocateMemory(VkMemoryRequirements reqs,
                       VkMemoryPropertyFlags properties) {
  auto memoryType =
      m_physicalDevice.findMemoryType(reqs.memoryTypeBits, properties);

  if (!memoryType) {
    return std::nullopt;
  }

  vk::info::MemoryAllocate info(reqs.size, memoryType->index);

  return DeviceMemory::create(*this, info, memoryType.value());
}

void Device::bindBufferMemory(Buffer &buffer, DeviceMemory &memory,
                              uint32_t offset) {
  vkBindBufferMemory(m_device, *buffer, *memory, offset);
}

std::optional<vk::DescriptorSetLayout>
Device::createDescriptorSetLayout(vk::info::DescriptorSetLayoutCreate &info) {
  return vk::DescriptorSetLayout::create(*this, info);
}

std::optional<vk::DescriptorPool>
Device::createDescriptorPool(vk::info::DescriptorPoolCreate &info) {
  return vk::DescriptorPool::create(*this, info);
}
} // namespace vk
