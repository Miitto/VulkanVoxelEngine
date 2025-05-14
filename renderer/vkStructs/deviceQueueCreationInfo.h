#pragma once

#include <print>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk {
class DeviceQueueCreateInfo : public VkDeviceQueueCreateInfo {
  std::vector<float> m_priorities;

public:
  DeviceQueueCreateInfo(uint32_t queueIndex, float priority = 1.0f)
      : DeviceQueueCreateInfo(queueIndex, std::vector<float>{priority}) {}

  DeviceQueueCreateInfo(uint32_t queueIndex, std::vector<float> priorities)
      : m_priorities(std::move(priorities)) {
    sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    pNext = nullptr;
    flags = 0;
    queueFamilyIndex = queueIndex;
    queueCount = static_cast<uint32_t>(m_priorities.size());
    pQueuePriorities = m_priorities.data();
  }

  DeviceQueueCreateInfo &setQueuePriorities(std::vector<float> priorities) {
    this->m_priorities = std::move(priorities);
    queueCount = static_cast<uint32_t>(m_priorities.size());
    pQueuePriorities = m_priorities.data();
    return *this;
  }

  DeviceQueueCreateInfo &addQueue(float priority) {
    m_priorities.push_back(priority);
    queueCount = static_cast<uint32_t>(m_priorities.size());
    pQueuePriorities = m_priorities.data();

    return *this;
  }
};
} // namespace vk
