module;

#include "vulkan/vulkan_core.h"

export module vk:bufferCopy;

export namespace vk {
class BufferCopy : public VkBufferCopy {
public:
  BufferCopy(uint32_t srcOffset = 0, uint32_t dstOffset = 0, uint32_t size = 0)
      : VkBufferCopy{
            .srcOffset = srcOffset, .dstOffset = dstOffset, .size = size} {}
  BufferCopy(const VkBufferCopy &o) : VkBufferCopy(o) {}

  BufferCopy &setSrcOffset(uint32_t offset) {
    srcOffset = offset;
    return *this;
  }

  BufferCopy &setDstOffset(uint32_t offset) {
    dstOffset = offset;
    return *this;
  }

  BufferCopy &setSize(uint32_t sz) {
    size = sz;
    return *this;
  }
};
} // namespace vk
