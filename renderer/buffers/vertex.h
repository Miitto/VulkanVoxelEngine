#pragma once

#include "buffers/buffer.h"

class VertexBuffer : public Buffer {
  VertexBuffer(VkBuffer buffer, Device &device, VkDeviceSize size) noexcept
      : Buffer(buffer, device, size) {}

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;

public:
  VertexBuffer(VertexBuffer &&o) noexcept : Buffer(std::move(o)) {}
  ~VertexBuffer() = default;

  static std::optional<VertexBuffer>
  create(Device &device, vk::info::VertexBufferCreate &createInfo);

  VkBuffer &operator*() { return m_buffer; }
  operator VkBuffer() { return m_buffer; }
};
