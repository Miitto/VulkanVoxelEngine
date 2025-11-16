#include <vkh/structs.hpp>

#include "vk-logger.hpp"

namespace vkh {
std::expected<AllocatedBuffer, std::string>
AllocatedBuffer::create(vma::Allocator &allocator,
                        const vk::BufferCreateInfo &bufInfo,
                        const vma::AllocationCreateInfo &allocInfo) {
  VMA_MAKE(buffer, allocator.createBuffer(bufInfo, allocInfo),
           "Failed to create allocated buffer");

  return AllocatedBuffer{
      .buffer = buffer.first,
      .alloc = buffer.second,
      .allocInfo = {},
  };
}
} // namespace vkh
