#pragma once

#include "device/device.h"
#include "device/memory.h"
#include "structs/MappedMemoryRange.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <span>

class Mapping {
  Device::Ref m_device;
  DeviceMemory::Ref m_memory;
  uint32_t m_size;
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

  Mapping(Device &device, DeviceMemory &memory, void *ptr, uint32_t size)
      : m_device(device.ref()), m_memory(memory.ref()), m_size(size),
        m_ptr(ptr), m_isCoherent(memory.isCoherent()) {}

public:
  static std::optional<Mapping> map(Device &device, DeviceMemory &memory,
                                    VkDeviceSize size, VkDeviceSize offset = 0,
                                    VkMemoryMapFlags flags = 0);

  Mapping(Mapping &&o) noexcept
      : m_device(std::move(o.m_device)), m_memory(std::move(o.m_memory)),
        m_size(o.m_size), m_ptr(o.m_ptr), m_isCoherent(o.m_isCoherent) {
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
    memcpy(m_ptr, static_cast<char *>(data) + offset, size);

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
