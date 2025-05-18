#pragma once

#include "vulkan/vulkan_core.h"

#include "device/device.h"

class DeviceMemory : public RawRefable<DeviceMemory, VkDeviceMemory> {
  VkDeviceMemory m_memory;
  Device::Ref m_device;

  VkDeviceSize m_size;

  PhysicalDevice::MemoryProperties m_memoryType;

  DeviceMemory() = delete;
  DeviceMemory(const DeviceMemory &) = delete;
  DeviceMemory &operator=(const DeviceMemory &) = delete;

public:
  using Ref = RawRef<DeviceMemory, VkDeviceMemory>;
  DeviceMemory(DeviceMemory &&o) noexcept
      : RawRefable(std::move(o)), m_memory(std::move(o.m_memory)),
        m_device(std::move(o.m_device)), m_size(o.m_size),
        m_memoryType(o.m_memoryType) {
    o.m_memory = VK_NULL_HANDLE;
  }

  DeviceMemory(Device &device, VkDeviceMemory memory, VkDeviceSize size,
               PhysicalDevice::MemoryProperties &memoryType)
      : RawRefable(), m_memory(memory), m_device(device.ref()), m_size(size),
        m_memoryType(memoryType) {}

  ~DeviceMemory() {
    if (m_memory != VK_NULL_HANDLE) {
      vkFreeMemory(**m_device, m_memory, nullptr);
      m_memory = VK_NULL_HANDLE;
    }
  }

  static std::optional<DeviceMemory>
  create(Device &device, VkMemoryAllocateInfo &info,
         PhysicalDevice::MemoryProperties &memoryType);

  VkDeviceMemory operator*() { return m_memory; }
  operator VkDeviceMemory() { return m_memory; }

  bool isCoherent() const {
    return (m_memoryType.memType.propertyFlags &
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
  }

  bool mappable() const {
    return (m_memoryType.memType.propertyFlags &
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
  }

  uint32_t getSize() const { return m_size; }
};
