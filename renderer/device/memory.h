#pragma once

#include "log.h"
#include "structs/MappedMemoryRange.h"
#include "vulkan/vulkan_core.h"

#include "core.h"
#include "device/device.h"

namespace vk {
class DeviceMemory;

class Mapping : public Refable<Mapping> {
  Device::Ref m_device;
  RawRef<DeviceMemory, VkDeviceMemory> m_memory;
  vk::DeviceSize m_offset;
  vk::DeviceSize m_size;
  void *m_ptr;

  bool m_isCoherent;

  struct Write {
    VkDeviceSize start;
    VkDeviceSize size;
  };

  std::vector<Write> m_writes = {};

  Mapping() = delete;
  Mapping(const Mapping &) = delete;
  Mapping &operator=(const Mapping &) = delete;
  Mapping operator=(Mapping &&o) = delete;

  Mapping(Device &device, DeviceMemory &memory, void *ptr,
          vk::DeviceSize offset, vk::DeviceSize size);

public:
  using Ref = Ref<Mapping>;
  static std::optional<Mapping> map(Device &device, DeviceMemory &memory,
                                    VkDeviceSize size, VkDeviceSize offset = 0,
                                    VkMemoryMapFlags flags = 0);

  operator bool() const {
    return m_ptr != nullptr && m_memory.has_value() && m_device.has_value();
  }

  Mapping(Mapping &&o) noexcept
      : Refable(std::move(o)), m_device(o.m_device), m_memory(o.m_memory),
        m_offset(o.m_offset), m_size(o.m_size), m_ptr(o.m_ptr),
        m_isCoherent(o.m_isCoherent) {
    o.m_ptr = nullptr;
    o.m_size = 0;
  }

  const void *get() const { return m_ptr; }
  uint32_t getSize() const { return m_size; }

  template <typename T> bool write(std::span<T> data, VkDeviceSize offset = 0) {
    VkDeviceSize size = data.size() * sizeof(T);
    if (offset + size > m_size) {
      return false;
    }

    writeUnchecked(data.data(), size, offset);
    return true;
  }

  bool write(void *data, uint32_t size, uint32_t offset = 0) {
    if (offset + size > m_size) {
      return false;
    }
    writeUnchecked(data, size, offset);
    return true;
  }

  void writeUnchecked(void *data, uint32_t size, uint32_t offset) {
    memcpy(static_cast<char *>(m_ptr) + offset, data, size);

    registerWrite({.start = offset, .size = size});
  }

  bool needsFlush() const { return !m_writes.empty(); }
  void registerWrite(Write write) {
    if (!m_isCoherent)
      m_writes.push_back(write);
  }

  void flush() {
    if (needsFlush()) {
      std::vector<vk::MappedMemoryRange> ranges;
      ranges.reserve(m_writes.size());
      for (auto &write : m_writes) {
        VkDeviceMemory memory = m_memory;
        ranges.emplace_back(memory, write.size, write.start);
      }
      vkFlushMappedMemoryRanges(m_device, ranges.size(), ranges.data());

      m_writes.clear();
    }
  }

  ~Mapping();
};

class MappingSegment {
  Mapping::Ref m_mapping;

  vk::DeviceSize m_offset;
  vk::DeviceSize m_size;

  MappingSegment() = delete;

public:
  MappingSegment(Mapping &mapping, vk::DeviceSize offset, vk::DeviceSize size)
      : m_mapping(mapping.ref()), m_offset(offset), m_size(size) {}

  MappingSegment(const MappingSegment &o)
      : m_mapping(o.m_mapping), m_offset(o.m_offset), m_size(o.m_size) {}
  MappingSegment &operator=(const MappingSegment &o) {
    if (this != &o) {
      m_mapping = o.m_mapping;
      m_offset = o.m_offset;
      m_size = o.m_size;
    }
    return *this;
  }

  MappingSegment(MappingSegment &&o) noexcept
      : m_mapping(o.m_mapping), m_offset(o.m_offset), m_size(o.m_size) {}
  MappingSegment &operator=(MappingSegment &&o) noexcept {
    if (this != &o) {
      m_mapping = o.m_mapping;
      m_offset = o.m_offset;
      m_size = o.m_size;
    }
    return *this;
  }

  bool write(void *data, uint32_t size, uint32_t offset = 0) {
    if (!m_mapping.has_value()) {
      LOG_ERR("Mapping is not valid");
      return false;
    }
    if (offset + size > m_size) {
      LOG_ERR("Write size {} exceeds segment size {}", size, m_size);
      return false;
    }
    auto &mapping = *m_mapping;

    mapping.writeUnchecked(data, size, m_offset + offset);
    return true;
  }
};

class DeviceMemory : public RawRefable<DeviceMemory, VkDeviceMemory> {
  VkDeviceMemory m_memory;
  Device::Ref m_device;

  VkDeviceSize m_size;

  PhysicalDevice::MemoryProperties m_memoryType;

  DeviceMemory() = delete;
  DeviceMemory(const DeviceMemory &) = delete;
  DeviceMemory &operator=(const DeviceMemory &) = delete;

  std::optional<Mapping::Ref> m_mapping;

protected:
  friend class Mapping;

  std::optional<Mapping::Ref> activeMapping() {
    if (m_mapping.has_value() && m_mapping.value().has_value()) {
      return m_mapping;
    } else {
      return std::nullopt;
    }
  }

public:
  using Ref = RawRef<DeviceMemory, VkDeviceMemory>;
  DeviceMemory(DeviceMemory &&o) noexcept
      : RawRefable(std::move(o)), m_memory(std::move(o.m_memory)),
        m_device(std::move(o.m_device)), m_size(o.m_size),
        m_memoryType(o.m_memoryType), m_mapping(o.m_mapping) {
    o.m_memory = VK_NULL_HANDLE;
  }

  DeviceMemory(Device &device, VkDeviceMemory memory, VkDeviceSize size,
               PhysicalDevice::MemoryProperties &memoryType)
      : RawRefable(), m_memory(memory), m_device(device.ref()), m_size(size),
        m_memoryType(memoryType) {}

  ~DeviceMemory() {
    if (m_memory != VK_NULL_HANDLE) {
      vkFreeMemory(**m_device, m_memory, nullptr);
      m_memory = VK_NULL_HANDLE;
    }
  }

  static std::optional<DeviceMemory>
  create(Device &device, VkMemoryAllocateInfo &info,
         PhysicalDevice::MemoryProperties &memoryType);

  VkDeviceMemory operator*() { return m_memory; }
  operator VkDeviceMemory() { return m_memory; }

  bool isCoherent() const {
    return (m_memoryType.memType.propertyFlags &
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
  }

  bool mappable() const {
    return (m_memoryType.memType.propertyFlags &
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
  }
  std::optional<Mapping> map(VkDeviceSize size = VK_WHOLE_SIZE,
                             VkDeviceSize offset = 0,
                             VkMemoryMapFlags flags = 0);

  uint32_t getSize() const { return m_size; }
};

} // namespace vk
