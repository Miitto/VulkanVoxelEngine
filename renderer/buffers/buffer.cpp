#include "buffer.h"
#include "log.h"

std::optional<Buffer> Buffer::create(Device &device,
                                     vk::info::BufferCreate &createInfo) {
  LOG("Creating buffer with size: {} and usage: {}", createInfo.size,
      createInfo.usage);
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

std::optional<Mapping> Buffer::map(VkDeviceSize size, VkDeviceSize offset,
                                   VkMemoryMapFlags flags) {
  if (!m_memory.has_value()) {
    std::cerr << "Buffer is not bound to memory." << std::endl;
    return std::nullopt;
  }

  auto memory = m_memory.value();

  if (!memory.memory.has_value()) {
    LOG_ERR("Buffer memory is not valid.");
    return std::nullopt;
  }

  if (!memory.memory.value().mappable()) {
    LOG_ERR("Buffer memory is not mappable.");
    return std::nullopt;
  }

  VkDeviceSize memorySize = (*memory.memory).getSize();

  // Ensure that the requested size and offset are within the bounds of the
  // buffer and the device memory
  if (memorySize + memory.offset <
      (size == VK_WHOLE_SIZE ? m_size : size) + offset) {
    LOG_ERR("Buffer memory is smaller than requested size.");
    return std::nullopt;
  }

  return Mapping::map(m_device, m_memory.value().memory, size, offset, flags);
}
