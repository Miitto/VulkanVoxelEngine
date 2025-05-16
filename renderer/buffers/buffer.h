#pragma once

#include "vulkan/vulkan_core.h"
#include <optional>

#include "device/device.h"

class Buffer {
  VkBuffer m_buffer;
  Device::Ref m_device;

  Buffer(VkBuffer buffer, Device &device) noexcept
      : m_buffer(buffer), m_device(device.ref()) {}

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

public:
  Buffer(Buffer &&o) noexcept : m_buffer(o.m_buffer), m_device(o.m_device) {
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

  VkMemoryRequirements getMemoryRequirements() {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(**m_device, m_buffer, &memoryRequirements);
    return memoryRequirements;
  }
};
