module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;

namespace vk {
std::optional<VertexBuffer>
VertexBuffer::create(Device &device, vk::info::VertexBufferCreate &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return VertexBuffer(buffer, device, createInfo.size, createInfo.usage);
}
} // namespace vk
