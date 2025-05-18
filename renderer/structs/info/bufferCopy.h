#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
class BufferCopy : public VkBufferCopy {
public:
  BufferCopy(uint32_t srcOffset = 0, uint32_t dstOffset = 0, uint32_t size = 0)
      : VkBufferCopy{
            .srcOffset = srcOffset, .dstOffset = dstOffset, .size = size} {}
  BufferCopy(const VkBufferCopy &o) : VkBufferCopy(o) {}

  BufferCopy &setSrcOffset(uint32_t srcOffset) {
    this->srcOffset = srcOffset;
    return *this;
  }

  BufferCopy &setDstOffset(uint32_t dstOffset) {
    this->dstOffset = dstOffset;
    return *this;
  }

  BufferCopy &setSize(uint32_t size) {
    this->size = size;
    return *this;
  }
};
} // namespace vk
