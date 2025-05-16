#include "mapping.h"

std::optional<Mapping> Mapping::map(Device &device, DeviceMemory &memory,
                                    VkDeviceSize size, VkDeviceSize offset,
                                    VkMemoryMapFlags flags) {
  void *ptr;
  if (vkMapMemory(*device, *memory, offset, size, flags, &ptr) != VK_SUCCESS) {
    return std::nullopt;
  }
  return Mapping(device, memory, ptr,
                 size == VK_WHOLE_SIZE ? memory.getSize() : size);
}

Mapping::~Mapping() {
  if (m_ptr != nullptr) {
    flush();
    vkUnmapMemory(m_device, m_memory);
  }
}
