#pragma once

#include "vulkan/vulkan_core.h"

#include "device/device.h"

class DeviceMemory {
public:
  class Ref {
    friend class DeviceMemory;
    std::shared_ptr<DeviceMemory *> m_memory;

    Ref() = delete;
    explicit Ref(DeviceMemory *memory)
        : m_memory(std::make_shared<DeviceMemory *>(memory)) {}

  protected:
    static Ref create(DeviceMemory &memory) { return Ref(&memory); }
    static Ref create(DeviceMemory *memory) { return Ref(memory); }

  public:
    void set(DeviceMemory *memory) { *this->m_memory = memory; }

    DeviceMemory &operator*() { return **m_memory; }
    operator DeviceMemory &() { return **m_memory; }
    operator VkDeviceMemory() { return ***m_memory; }
  };

private:
  VkDeviceMemory m_memory;
  Device::Ref m_device;
  Ref m_reference;

  VkDeviceSize m_size;
  bool m_isCoherent;

  DeviceMemory() = delete;
  DeviceMemory(const DeviceMemory &) = delete;
  DeviceMemory &operator=(const DeviceMemory &) = delete;

public:
  DeviceMemory(DeviceMemory &&o) noexcept
      : m_memory(std::move(o.m_memory)), m_device(std::move(o.m_device)),
        m_reference(o.m_reference), m_size(o.m_size),
        m_isCoherent(o.m_isCoherent) {
    o.m_memory = VK_NULL_HANDLE;
    m_reference.set(this);
  }

  DeviceMemory(Device &device, VkDeviceMemory memory, VkDeviceSize size,
               bool coherent)
      : m_memory(memory), m_device(device.ref()),
        m_reference(Ref::create(*this)), m_size(size), m_isCoherent(coherent) {}

  ~DeviceMemory() {
    if (m_memory != VK_NULL_HANDLE) {
      vkFreeMemory(**m_device, m_memory, nullptr);
      m_memory = VK_NULL_HANDLE;
    }
  }

  static std::optional<DeviceMemory> create(Device &device,
                                            VkMemoryAllocateInfo &info);

  VkDeviceMemory operator*() { return m_memory; }
  operator VkDeviceMemory() { return m_memory; }
  Ref &ref() { return m_reference; }

  bool isCoherent() const { return m_isCoherent; }
  uint32_t getSize() const { return m_size; }
};
