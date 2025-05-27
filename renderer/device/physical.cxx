module;

#include <cstring>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

namespace vk {
std::vector<PhysicalDevice> PhysicalDevice::all(Instance &instance) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    return {};
  }
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

  std::vector<PhysicalDevice> physicalDevices;
  for (auto device : devices) {
    physicalDevices.emplace_back(PhysicalDevice(device));
  }
  return physicalDevices;
}

VkPhysicalDeviceProperties PhysicalDevice::getProperties() const {
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(device, &props);
  return props;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures() const {
  VkPhysicalDeviceFeatures feats;
  vkGetPhysicalDeviceFeatures(device, &feats);
  return feats;
}

std::vector<VkExtensionProperties> PhysicalDevice::getExtensions() const {
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       extensions.data());
  return extensions;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::getMemoryProperties() const {
  VkPhysicalDeviceMemoryProperties props;
  vkGetPhysicalDeviceMemoryProperties(device, &props);
  return props;
}

std::vector<QueueFamily> PhysicalDevice::getQueues() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           families.data());

  std::vector<QueueFamily> queueFamilies;
  for (size_t i = 0; i < queueFamilyCount; ++i) {
    queueFamilies.emplace_back(*this, families[i], static_cast<uint32_t>(i));
  }

  return queueFamilies;
}

std::vector<char const *> PhysicalDevice::findUnsupportedExtensions(
    std::vector<char const *> extensions) const {
  std::vector<VkExtensionProperties> availableExtensions = getExtensions();
  std::vector<char const *> unsupportedExtensions;

  for (auto requested : extensions) {
    bool found = false;
    for (const auto &extension : availableExtensions) {
      if (strcmp(requested, extension.extensionName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      unsupportedExtensions.push_back(requested);
    }
  }

  return unsupportedExtensions;
}

std::optional<PhysicalDevice::MemoryProperties>
PhysicalDevice::findMemoryType(uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties memProperties = getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return PhysicalDevice::MemoryProperties{i, memProperties.memoryTypes[i]};
    }
  }

  return std::nullopt;
}
} // namespace vk
