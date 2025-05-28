module;

#include "refs/refable.h"
#include <optional>
#include <span>
#include <vulkan/vulkan_core.h>

export module vk:device;

import :device.physical;
import :info.buffers.create;
import :info.deviceCreate;

namespace vk {
export class Swapchain;
export class Queue;
export class CommandPool;
export class Semaphore;
export class Fence;
export class Buffer;
export class DeviceMemory;
export class VertexBuffer;
export class IndexBuffer;
export class UniformBuffer;

export class DescriptorSetLayout;
export class DescriptorPool;

namespace info {
export class DescriptorSetLayoutCreate;
export class DescriptorPoolCreate;
export class SwapchainCreate;
export class CommandPoolCreate;
} // namespace info
} // namespace vk

namespace vk {
export class Device : public RawRefable<Device, VkDevice> {
  VkDevice m_device;
  PhysicalDevice m_physicalDevice;

public:
  using Ref = RawRef<Device, VkDevice>;

  Device(VkDevice device, PhysicalDevice &physicalDevice)
      : RawRefable(), m_device(device), m_physicalDevice(physicalDevice) {}
  void destroy() {
    if (m_device != VK_NULL_HANDLE) {
      waitIdle();
      vkDestroyDevice(m_device, nullptr);
      m_device = VK_NULL_HANDLE;
    }
  }
  ~Device() { destroy(); }

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device &operator=(Device &&o) = delete;

  Device(Device &&o) noexcept
      : RawRefable(std::move(o)), m_device(std::move(o.m_device)),
        m_physicalDevice(std::move(o.m_physicalDevice)) {
    o.m_device = VK_NULL_HANDLE;
  }

  VkDevice &operator*() { return m_device; }
  operator VkDevice() { return m_device; }

  static std::optional<Device>
  create(PhysicalDevice &physicalDevice,
         vk::info::DeviceCreate &createInfo) noexcept;

  PhysicalDevice &getPhysical() { return m_physicalDevice; }

  std::optional<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);

  VkResult waitIdle() { return vkDeviceWaitIdle(m_device); }

  std::optional<CommandPool>
  createCommandPool(vk::info::CommandPoolCreate &info);
  std::optional<Swapchain> createSwapchain(vk::info::SwapchainCreate &info);
  std::optional<Semaphore> createSemaphore();
  std::optional<Fence> createFence(bool createSignaled = false);

  std::optional<Buffer> createBuffer(vk::info::BufferCreate &info);
  std::optional<VertexBuffer>
  createVertexBuffer(vk::info::VertexBufferCreate &info);
  std::optional<IndexBuffer>
  createIndexBuffer(vk::info::IndexBufferCreate &info);
  std::optional<UniformBuffer>
  createUniformBuffer(vk::info::UniformBufferCreate &info);

  std::optional<DeviceMemory> allocateMemory(Buffer &buffer,
                                             VkMemoryPropertyFlags properties);

  std::optional<DeviceMemory> allocateMemory(std::span<Buffer *> buffers,
                                             VkMemoryPropertyFlags properties);

  std::optional<DeviceMemory> allocateMemory(VkMemoryRequirements memReqs,
                                             VkMemoryPropertyFlags porperties);

  void bindBufferMemory(Buffer &buffer, DeviceMemory &memory,
                        uint32_t offset = 0);

  std::optional<vk::DescriptorSetLayout>
  createDescriptorSetLayout(vk::info::DescriptorSetLayoutCreate &createInfo);
  std::optional<DescriptorPool>
  createDescriptorPool(info::DescriptorPoolCreate &createInfo);
};
} // namespace vk
