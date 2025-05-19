#pragma once

#include "buffers/buffer.h"
#include <vulkan/vulkan_core.h>

class IndexBuffer : public Buffer {
  IndexBuffer(VkBuffer buffer, Device &device, VkDeviceSize size,
              VkBufferUsageFlags usage) noexcept
      : Buffer(buffer, device, size, usage) {}

  IndexBuffer(const IndexBuffer &) = delete;
  IndexBuffer &operator=(const IndexBuffer &) = delete;

public:
  IndexBuffer(IndexBuffer &&o) noexcept : Buffer(std::move(o)) {}
  ~IndexBuffer() = default;

  static std::optional<IndexBuffer>
  create(Device &device, vk::info::IndexBufferCreate &createInfo);

  VkBuffer &operator*() { return m_buffer; }
  operator VkBuffer() { return m_buffer; }
};
