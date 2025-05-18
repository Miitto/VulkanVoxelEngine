#pragma once
#include "vulkan/vulkan_core.h"

namespace vk {
namespace info {

class BufferCreate : public VkBufferCreateInfo {

public:
  BufferCreate()
      : VkBufferCreateInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                           .pNext = nullptr,
                           .flags = 0,
                           .size = 0,
                           .usage = 0,
                           .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                           .queueFamilyIndexCount = 0,
                           .pQueueFamilyIndices = nullptr} {}
  BufferCreate(VkBufferUsageFlags usage, uint32_t size,
               VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
      : BufferCreate() {
    this->usage = usage;
    this->size = size;
    this->sharingMode = sharingMode;
  }
};

class VertexBufferCreate : public BufferCreate {
public:
  VertexBufferCreate() : BufferCreate() {
    usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }

  VertexBufferCreate(uint32_t size, VkBufferUsageFlags additionalUsage = 0,
                     VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
      : BufferCreate(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | additionalUsage, size,
                     sharingMode) {}
};
} // namespace info
} // namespace vk
