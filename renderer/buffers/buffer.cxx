module;
#include "log.h"
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

namespace vk {
std::optional<Buffer> Buffer::create(Device &device,
                                     vk::info::BufferCreate &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return Buffer(buffer, device, createInfo.size, createInfo.usage);
}

VkResult Buffer::bind(DeviceMemory &memory, VkDeviceSize offset) {
  auto res = vkBindBufferMemory(**m_device, m_buffer, memory, offset);

  if (res != VK_SUCCESS) {
    return res;
  }

  m_memory = {memory.ref(), offset};

  return res;
}

} // namespace vk
