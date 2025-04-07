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
  vkGetPhysicalDeviceProperties(device, &properties);
  this->properties = properties;
  return properties;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures() {
  if (features.has_value()) {
    return *features;
  }
  VkPhysicalDeviceFeatures features;
  vkGetPhysicalDeviceFeatures(device, &features);
  this->features = features;
  return features;
}
