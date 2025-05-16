#pragma once

#include "mapping.h"
#include "vulkan/vulkan_core.h"
#include <optional>

#include "device/device.h"
#include "device/memory.h"

class Buffer {
protected:
  VkBuffer m_buffer;
  Device::Ref m_device;
  VkDeviceSize m_size;

  struct MemoryLocation {
    DeviceMemory::Ref memory;
    VkDeviceSize offset;
  };

  std::optional<MemoryLocation> m_memory;

  Buffer(VkBuffer buffer, Device &device, VkDeviceSize size) noexcept
      : m_buffer(buffer), m_device(device.ref()), m_size(size) {}

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

public:
  Buffer(Buffer &&o) noexcept
      : m_buffer(o.m_buffer), m_device(o.m_device), m_size(o.m_size),
        m_memory(std::move(o.m_memory)) {
    o.m_buffer = VK_NULL_HANDLE;
  }
  ~Buffer() {
    if (m_buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(**m_device, m_buffer, nullptr);
    }
  }

  static std::optional<Buffer> create(Device &device,
                                      VkBufferCreateInfo &createInfo);

  VkBuffer &operator*() { return m_buffer; }
  operator VkBuffer() { return m_buffer; }

  VkMemoryRequirements getMemoryRequirements() {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(**m_device, m_buffer, &memoryRequirements);
    return memoryRequirements;
  }

  VkResult bind(DeviceMemory &memory, VkDeviceSize offset = 0);
  std::optional<Mapping> map(VkDeviceSize size = VK_WHOLE_SIZE,
                             VkDeviceSize offset = 0,
                             VkMemoryMapFlags flags = 0);
};
