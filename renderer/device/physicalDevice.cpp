#include "physicalDevice.h"
#include "queue.h"
#include <cstring>
#include <vector>

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

VkPhysicalDeviceProperties PhysicalDevice::getProperties() {
  if (properties.has_value()) {
    return *properties;
  }

  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(device, &props);
  properties = props;
  return props;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures() {
  if (features.has_value()) {
    return *features;
  }
  VkPhysicalDeviceFeatures feats;
  vkGetPhysicalDeviceFeatures(device, &feats);
  this->features = feats;
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
