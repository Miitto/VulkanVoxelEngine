#include "index.h"

std::optional<IndexBuffer>
IndexBuffer::create(Device &device, vk::info::IndexBufferCreate &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return IndexBuffer(buffer, device, createInfo.size, createInfo.usage);
}
