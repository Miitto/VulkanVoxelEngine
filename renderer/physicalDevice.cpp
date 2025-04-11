#include "physicalDevice.h"

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
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(**device, &properties);
  this->properties = properties;
  return properties;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures() {
  if (features.has_value()) {
    return *features;
  }
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(**device, &features);
  this->features = features;
  return features;
}

std::vector<VkExtensionProperties> PhysicalDevice::getExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(**device, nullptr, &extensionCount,
                                       nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(**device, nullptr, &extensionCount,
                                       extensions.data());
  return extensions;
}

std::vector<char const *> PhysicalDevice::findUnsupportedExtensions(
    std::vector<char const *> extensions) {
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
