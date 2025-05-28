module;

#include "log.h"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

namespace vk {
Mapping::Mapping(Device &device, DeviceMemory &memory, void *ptr,
                 vk::DeviceSize offset, vk::DeviceSize size)
    : Refable(), m_device(device.ref()), m_memory(memory.ref()),
      m_offset(offset), m_size(size), m_ptr(ptr),
      m_isCoherent(memory.isCoherent()) {}

std::optional<DeviceMemory>
DeviceMemory::create(Device &device, VkMemoryAllocateInfo &info,
                     PhysicalDevice::MemoryProperties &memoryType) {
  VkDeviceMemory memory;
  if (vkAllocateMemory(*device, &info, nullptr, &memory) != VK_SUCCESS) {
    return std::nullopt;
  }

  return DeviceMemory(device, memory, info.allocationSize, memoryType);
}

std::optional<Mapping> Mapping::map(Device &device, DeviceMemory &memory,
                                    VkDeviceSize size, VkDeviceSize offset,
                                    VkMemoryMapFlags flags) {
  auto activeMapping = memory.activeMapping();
  if (activeMapping.has_value()) {
    LOG_ERR("Memory is already mapped");
    return std::nullopt;
  }

  void *ptr;
  if (vkMapMemory(*device, *memory, offset, size, flags, &ptr) != VK_SUCCESS) {
    return std::nullopt;
  }
  return Mapping(device, memory, ptr, offset,
                 size == VK_WHOLE_SIZE ? memory.getSize() : size);
}

Mapping::~Mapping() {
  if (m_ptr != nullptr) {
    flush();
    vkUnmapMemory(m_device, m_memory);
    m_ptr = nullptr;
  }
}

std::optional<Mapping> DeviceMemory::map(VkDeviceSize size, VkDeviceSize offset,
                                         VkMemoryMapFlags flags) {
  if (!m_memory) {
    LOG_ERR("Memory is not valid.");
    return std::nullopt;
  }

  if (!mappable()) {
    LOG_ERR("Memory is not mappable.");
    return std::nullopt;
  }

  // Ensure that the requested size and offset are within the bounds of the
  // buffer and the device memory
  if (m_size < (size == VK_WHOLE_SIZE ? m_size : size) + offset) {
    LOG_ERR("Memory is smaller than requested size.");
    return std::nullopt;
  }

  return Mapping::map(m_device, *this, size, offset, flags);
}

} // namespace vk
