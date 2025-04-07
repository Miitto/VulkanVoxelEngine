#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class DeviceCreateInfoBuilder {
  VkDeviceCreateInfo createInfo;
  VkPhysicalDeviceFeatures features;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

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
    queueCreateInfos.push_back(queueCreateInfo);
    return *this;
  }

  VkDeviceCreateInfo build() {
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    return createInfo;
  }
};
