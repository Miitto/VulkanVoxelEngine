module;

#include "vulkan/vulkan_core.h"
#include <optional>

export module vk:buffer;

import :structs.info.buffers.create;
import :device;
import :memory;

export namespace vk {
class Buffer {
protected:
  VkBuffer m_buffer;
  Device::Ref m_device;
  VkDeviceSize m_size;
  VkBufferUsageFlags m_usage;

  struct MemoryLocation {
    DeviceMemory::Ref memory;
    VkDeviceSize offset;
  };

  std::optional<MemoryLocation> m_memory;

  Buffer(VkBuffer buffer, Device &device, VkDeviceSize size,
         VkBufferUsageFlags usage) noexcept
      : m_buffer(buffer), m_device(device.ref()), m_size(size), m_usage(usage) {
  }

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

public:
  Buffer(Buffer &&o) noexcept
      : m_buffer(o.m_buffer), m_device(o.m_device), m_size(o.m_size),
        m_usage(o.m_usage), m_memory(std::move(o.m_memory)) {
    o.m_buffer = VK_NULL_HANDLE;
  }
  Buffer &operator=(Buffer &&o) noexcept {
    if (this != &o) {
      m_buffer = o.m_buffer;
      m_device = o.m_device;
      m_size = o.m_size;
      m_usage = o.m_usage;
      m_memory = o.m_memory;
      o.m_buffer = VK_NULL_HANDLE;
    }
    return *this;
  }
  ~Buffer() {
    if (m_buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(**m_device, m_buffer, nullptr);
    }
  }

  static std::optional<Buffer> create(Device &device,
                                      vk::info::BufferCreate &createInfo);

  VkBuffer &operator*() { return m_buffer; }
  operator VkBuffer() { return m_buffer; }

  VkDeviceSize size() { return m_size; }

  Device::Ref &getDevice() { return m_device; }

  VkMemoryRequirements getMemoryRequirements() {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(**m_device, m_buffer, &memoryRequirements);
    return memoryRequirements;
  }

  VkResult bind(DeviceMemory &memory, VkDeviceSize offset = 0);

  bool isBound() const {
    return m_memory.has_value() && m_memory.value().memory.has_value();
  }

  bool canMap() const {
    return isBound() && m_memory.value().memory.value().mappable();
  }

  bool canCopyFrom() const {
    return isBound() && m_usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  bool canCopyTo() const {
    return isBound() && (m_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) != 0;
  }
};
} // namespace vk
