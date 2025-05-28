module;

#include <vector>
#include <vulkan/vulkan_core.h>

export module vk:info.deviceQueueCreate;

export namespace vk {
namespace info {
class DeviceQueueCreate : public VkDeviceQueueCreateInfo {
  std::vector<float> m_priorities;

  void setupPriorities() {
    queueCount = static_cast<uint32_t>(m_priorities.size());
    pQueuePriorities = m_priorities.empty() ? nullptr : m_priorities.data();
  }

public:
  DeviceQueueCreate(uint32_t queueIndex, float priority = 1.0f)
      : DeviceQueueCreate(queueIndex, std::vector<float>{priority}) {}

  DeviceQueueCreate(uint32_t queueIndex, std::vector<float> priorities)
      : VkDeviceQueueCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .queueFamilyIndex = queueIndex,
                                .queueCount = 0,
                                .pQueuePriorities = nullptr},
        m_priorities(std::move(priorities)) {
    setupPriorities();
  }

  DeviceQueueCreate &setQueuePriorities(std::vector<float> priorities) {
    this->m_priorities = std::move(priorities);
    setupPriorities();
    return *this;
  }

  DeviceQueueCreate &addQueue(float priority) {
    m_priorities.push_back(priority);
    setupPriorities();

    return *this;
  }

  DeviceQueueCreate(const DeviceQueueCreate &other)
      : VkDeviceQueueCreateInfo{other}, m_priorities(other.m_priorities) {
    setupPriorities();
  }

  DeviceQueueCreate(DeviceQueueCreate &&other) noexcept
      : VkDeviceQueueCreateInfo{other},
        m_priorities(std::move(other.m_priorities)) {
    setupPriorities();
    other.setupPriorities();
  }
};
} // namespace info
} // namespace vk
