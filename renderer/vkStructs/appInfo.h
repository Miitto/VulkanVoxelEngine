#pragma once
#include <vulkan/vulkan.h>

class ApplicationInfoBuilder {
  VkApplicationInfo appInfo;

public:
  ApplicationInfoBuilder() {
    appInfo.pApplicationName = "Vulkan App";
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
  }

  ApplicationInfoBuilder &setApplicationName(const char *name) {
    appInfo.pApplicationName = name;
    return *this;
  }

  ApplicationInfoBuilder &setApplicationVersion(uint32_t version) {
    appInfo.applicationVersion = version;
    return *this;
  }

  ApplicationInfoBuilder &setApplicationVersion(uint32_t major, uint32_t minor,
                                                uint32_t patch) {
    appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    return *this;
  }

  ApplicationInfoBuilder &setEngineVersion(uint32_t version) {
    appInfo.engineVersion = version;
    return *this;
  }

  ApplicationInfoBuilder &setEngineName(const char *name) {
    appInfo.pEngineName = name;
    return *this;
  }

  ApplicationInfoBuilder &setApiVersion(uint32_t version) {
    appInfo.apiVersion = version;
    return *this;
  }

  const VkApplicationInfo build() { return appInfo; }
};
