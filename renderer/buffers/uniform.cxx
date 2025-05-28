module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;

namespace vk {
std::optional<UniformBuffer>
UniformBuffer::create(Device &device,
                      vk::info::UniformBufferCreate &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return UniformBuffer(buffer, device, createInfo.size, createInfo.usage);
}
} // namespace vk
