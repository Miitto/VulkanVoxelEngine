#include "device.h"
#include "buffers/buffer.h"
#include "buffers/vertex.h"
#include "device.h"
#include "device/memory.h"
#include "structs/info/memoryAllocate.h"
#include "swapchain.h"
#include "sync/fence.h"
#include "sync/semaphore.h"
#include <optional>

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

std::optional<DeviceMemory>
Device::allocateMemory(Buffer &buffer, VkMemoryPropertyFlags properties) {
  auto memoryReqs = buffer.getMemoryRequirements();

  auto memoryTypeIndex = m_physicalDevice.findMemoryTypeIndex(
      memoryReqs.memoryTypeBits, properties);

  if (!memoryTypeIndex) {
    return std::nullopt;
  }

  vk::info::MemoryAllocate info(memoryReqs.size, *memoryTypeIndex);

  return DeviceMemory::create(*this, info);
}

void Device::bindBufferMemory(Buffer &buffer, DeviceMemory &memory,
                              uint32_t offset) {
  vkBindBufferMemory(m_device, *buffer, *memory, offset);
}
