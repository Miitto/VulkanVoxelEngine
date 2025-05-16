#include "vertex.h"

std::optional<VertexBuffer>
VertexBuffer::create(Device &device, vk::info::VertexBufferCreate &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return VertexBuffer(buffer, device, createInfo.size);
}
