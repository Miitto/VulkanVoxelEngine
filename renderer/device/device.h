#pragma once

#include "physicalDevice.h"
#include "queue.h"
#include <iostream>
#include <optional>
#include <vulkan/vulkan.h>

class Device {
public:
  class Ref {
    friend class Device;
    std::shared_ptr<Device *> device;

    Ref() = delete;
    explicit Ref(Device *device) : device(std::make_shared<Device *>(device)) {}

  protected:
    static Ref create(Device &device) { return Ref(&device); }
    static Ref create(Device *device) { return Ref(device); }

  public:
    void set(Device *device) { *this->device = device; }

    Device &operator*() { return **device; }
    VkDevice &raw() { return ***device; }
  };

private:
  VkDevice device;
  Ref reference;

public:
  Device(VkDevice device) : device(device), reference(Ref::create(this)) {}
  void destroy() {
    if (device != VK_NULL_HANDLE) {
      waitIdle();
      vkDestroyDevice(device, nullptr);
      device = VK_NULL_HANDLE;
    }
  }
  ~Device() { destroy(); }

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device &operator=(Device &&o) = delete;

  Device(Device &&o) noexcept
      : device(std::move(o.device)), reference(o.reference) {
    o.device = VK_NULL_HANDLE;
    reference.set(this);
  }

  VkDevice &operator*() { return device; }
  Ref &ref() { return reference; }

  static std::optional<Device> create(PhysicalDevice &physicalDevice,
                                      VkDeviceCreateInfo &createInfo);

  std::optional<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);

  VkResult waitIdle() { return vkDeviceWaitIdle(device); }
};
