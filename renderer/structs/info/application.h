#pragma once
#include "core.h"
#include <vulkan/vulkan.h>

namespace vk {
namespace info {

class Application : public VkApplicationInfo {
public:
  Application(const char *name = "Vulkan App", Version version = {1, 0, 0},
              const char *engineName = "No Engine",
              Version engineVersion = {1, 0, 0}) {
    pApplicationName = name;
    sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    pNext = nullptr;
    applicationVersion = version;
    pEngineName = engineName;
    this->engineVersion = engineVersion;
    apiVersion = VK_API_VERSION_1_0;
  }
};
} // namespace info
} // namespace vk
