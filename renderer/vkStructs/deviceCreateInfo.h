#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class DeviceCreateInfoBuilder {
  VkDeviceCreateInfo createInfo;
  VkPhysicalDeviceFeatures features;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::vector<char const *> extensions;

public:
  DeviceCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = 0;
    createInfo.pQueueCreateInfos = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.pEnabledFeatures = nullptr;
  }

  DeviceCreateInfoBuilder &
  setPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures &features) {
    this->features = features;
    return *this;
  }

  DeviceCreateInfoBuilder &
  addQueueCreateInfo(const VkDeviceQueueCreateInfo &queueCreateInfo) {
    auto queueIdx = queueCreateInfo.queueFamilyIndex;
    auto queueCount = queueCreateInfo.queueCount;

    for (auto &info : queueCreateInfos) {
      if (info.queueFamilyIndex == queueIdx) {
        return *this;
      }
    }

    queueCreateInfos.push_back(queueCreateInfo);
    return *this;
  }

  DeviceCreateInfoBuilder &enableExtension(const char *extension) {
    extensions.push_back(extension);
    return *this;
  }

  VkDeviceCreateInfo build() {
    createInfo.pEnabledFeatures = &features;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    return createInfo;
  }
};
