#include "buffer.h"

std::optional<Buffer> Buffer::create(Device &device,
                                     VkBufferCreateInfo &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return Buffer(buffer, device);
}
