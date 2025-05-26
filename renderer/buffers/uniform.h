#pragma once

#include "buffers/buffer.h"
#include <vulkan/vulkan_core.h>

namespace vk {
class UniformBuffer : public Buffer {
  UniformBuffer(VkBuffer buffer, Device &device, VkDeviceSize size,
                VkBufferUsageFlags usage) noexcept
      : Buffer(buffer, device, size, usage) {}

  UniformBuffer(const UniformBuffer &) = delete;
  UniformBuffer &operator=(const UniformBuffer &) = delete;

public:
  UniformBuffer(UniformBuffer &&o) noexcept : Buffer(std::move(o)) {}
  UniformBuffer &operator=(UniformBuffer &&o) noexcept {
    if (this != &o) {
      Buffer::operator=(std::move(o));
    }
    return *this;
  }
  ~UniformBuffer() = default;

  static std::optional<UniformBuffer>
  create(Device &device, vk::info::UniformBufferCreate &createInfo);

  VkBuffer &operator*() { return m_buffer; }
  operator VkBuffer() { return m_buffer; }
};
} // namespace vk
