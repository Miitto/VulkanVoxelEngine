module;

#include <optional>
#include <vulkan/vulkan_core.h>

export module vk:buffers.vertex;

import :buffer;

export namespace vk {
class VertexBuffer : public Buffer {
  VertexBuffer(VkBuffer buffer, Device &device, VkDeviceSize size,
               VkBufferUsageFlags usage) noexcept
      : Buffer(buffer, device, size, usage) {}

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
} // namespace vk
