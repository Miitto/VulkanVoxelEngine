#pragma once

#include "vulkan/vulkan_core.h"

#include "device/device.h"

class DeviceMemory {
  VkDeviceMemory m_memory;
  Device::Ref m_device;

  DeviceMemory() = delete;
  DeviceMemory(const DeviceMemory &) = delete;
  DeviceMemory &operator=(const DeviceMemory &) = delete;

public:
  DeviceMemory(DeviceMemory &&o) noexcept
      : m_memory(std::move(o.m_memory)), m_device(std::move(o.m_device)) {
    o.m_memory = VK_NULL_HANDLE;
  }

  DeviceMemory(Device &device, VkDeviceMemory memory)
      : m_memory(memory), m_device(device.ref()) {}

  ~DeviceMemory() {
    if (m_memory != VK_NULL_HANDLE) {
      vkFreeMemory(**m_device, m_memory, nullptr);
      m_memory = VK_NULL_HANDLE;
    }
  }

  static std::optional<DeviceMemory> create(Device &device,
                                            VkMemoryAllocateInfo &info);

  VkDeviceMemory operator*() { return m_memory; }
};
