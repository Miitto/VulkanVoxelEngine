#pragma once

#include "vkStructs/deviceQueueCreationInfo.h"
#include <concepts>
#include <print>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk {
class DeviceCreateInfo : public VkDeviceCreateInfo {
  VkPhysicalDeviceFeatures m_features{};
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
  std::vector<char const *> extensions{};

  void setupQueues() {
    queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    pQueueCreateInfos = queueCreateInfos.data();
  }

  void setupExtensions() {
    enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ppEnabledExtensionNames = extensions.data();
  }

public:
  DeviceCreateInfo() {
    sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    pNext = nullptr;
    flags = 0;
    queueCreateInfoCount = 0;
    pQueueCreateInfos = nullptr;
    enabledLayerCount = 0;
    ppEnabledLayerNames = nullptr;
    enabledExtensionCount = 0;
    ppEnabledExtensionNames = nullptr;
    pEnabledFeatures = nullptr;
  }

  DeviceCreateInfo(const VkPhysicalDeviceFeatures &features)
      : DeviceCreateInfo() {
    m_features = features;
    pEnabledFeatures = &m_features;
  }

  DeviceCreateInfo &
  setPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures &features) {
    m_features = features;
    pEnabledFeatures = &m_features;

    return *this;
  }

  DeviceCreateInfo &addQueue(const VkDeviceQueueCreateInfo &queueCreateInfo) {
    auto queueIdx = queueCreateInfo.queueFamilyIndex;

    for (auto &info : queueCreateInfos) {
      if (info.queueFamilyIndex == queueIdx) {
        return *this;
      }
    }

    queueCreateInfos.push_back(queueCreateInfo);

    setupQueues();
    return *this;
  }

  DeviceCreateInfo &enableExtension(const char *extension) {
    extensions.push_back(extension);

    setupExtensions();

    return *this;
  }
};
} // namespace vk
