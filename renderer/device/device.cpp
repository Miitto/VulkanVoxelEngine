#include "device.h"
#include "swapchain.h"
#include "sync/fence.h"
#include "sync/semaphore.h"

std::optional<Device> Device::create(PhysicalDevice &physicalDevice,
                                     VkDeviceCreateInfo &createInfo) {

  VkDevice device;
  VkResult result =
      vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }
  return Device(device);
}

std::optional<Queue> Device::getQueue(uint32_t queueFamilyIndex,
                                      uint32_t queueIndex) {
  VkQueue queue;
  vkGetDeviceQueue(m_device, queueFamilyIndex, queueIndex, &queue);
  if (queue == VK_NULL_HANDLE) {
    return std::nullopt;
  }
  return Queue(queue, queueFamilyIndex);
}

std::optional<Swapchain>
Device::createSwapchain(VkSwapchainCreateInfoKHR &info) {
  return Swapchain::create(*this, info);
}

std::optional<Semaphore> Device::createSemaphore() {
  return Semaphore::create(*this);
}

std::optional<Fence> Device::createFence(bool signaled) {
  return Fence::create(*this, signaled);
}
