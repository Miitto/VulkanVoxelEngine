#pragma once

#include "physicalDevice.h"
#include "queue.h"
#include <optional>
#include <vulkan/vulkan.h>

class Swapchain;
class Semaphore;
class Fence;

class Device {
public:
  class Ref {
    friend class Device;
    std::shared_ptr<Device *> m_device;

    Ref() = delete;
    explicit Ref(Device *device)
        : m_device(std::make_shared<Device *>(device)) {}

  protected:
    static Ref create(Device &device) { return Ref(&device); }
    static Ref create(Device *device) { return Ref(device); }

  public:
    void set(Device *device) { *this->m_device = device; }

    Device &operator*() { return **m_device; }
    VkDevice &raw() { return ***m_device; }
  };

private:
  VkDevice m_device;
  Ref m_reference;

public:
  Device(VkDevice device) : m_device(device), m_reference(Ref::create(this)) {}
  void destroy() {
    if (m_device != VK_NULL_HANDLE) {
      waitIdle();
      vkDestroyDevice(m_device, nullptr);
      m_device = VK_NULL_HANDLE;
    }
  }
  ~Device() { destroy(); }

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device &operator=(Device &&o) = delete;

  Device(Device &&o) noexcept
      : m_device(std::move(o.m_device)), m_reference(o.m_reference) {
    o.m_device = VK_NULL_HANDLE;
    m_reference.set(this);
  }

  VkDevice &operator*() { return m_device; }
  Ref &ref() { return m_reference; }

  static std::optional<Device> create(PhysicalDevice &physicalDevice,
                                      VkDeviceCreateInfo &createInfo);

  std::optional<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);

  VkResult waitIdle() { return vkDeviceWaitIdle(m_device); }

  std::optional<Swapchain> createSwapchain(VkSwapchainCreateInfoKHR &info);
  std::optional<Semaphore> createSemaphore();
  std::optional<Fence> createFence(bool createSignaled = false);
};
