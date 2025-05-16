#include "buffer.h"

std::optional<Buffer> Buffer::create(Device &device,
                                     VkBufferCreateInfo &createInfo) {
  VkBuffer buffer;
  if (vkCreateBuffer(*device, &createInfo, nullptr, &buffer) != VK_SUCCESS) {
    return std::nullopt;
  }

  return Buffer(buffer, device, createInfo.size);
}

VkResult Buffer::bind(DeviceMemory &memory, VkDeviceSize offset) {
  auto res = vkBindBufferMemory(**m_device, m_buffer, memory, offset);

  if (res != VK_SUCCESS) {
    return res;
  }

  m_memory = {memory.ref(), offset};

  return res;
}

std::optional<Mapping> Buffer::map(VkDeviceSize size, VkDeviceSize offset,
                                   VkMemoryMapFlags flags) {
  if (!m_memory.has_value()) {
    std::cerr << "Buffer is not bound to memory." << std::endl;
    return std::nullopt;
  }

  auto memory = m_memory.value();

  if (*memory.memory == VK_NULL_HANDLE) {
    std::cerr << "Buffer memory is null." << std::endl;
    return std::nullopt;
  }

  VkDeviceSize bufferSize = (*memory.memory).getSize();

  // Ensure that the requested size and offset are within the bounds of the
  // buffer and the device memory
  if (bufferSize + memory.offset <
      (size == VK_WHOLE_SIZE ? m_size : size) + offset) {
    std::cerr << "Buffer memory is smaller than requested size." << std::endl;
    return std::nullopt;
  }

  return Mapping::map(m_device, m_memory.value().memory, size, offset, flags);
}
