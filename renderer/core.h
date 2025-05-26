#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
using DeviceSize = VkDeviceSize;
using Result = VkResult;

struct Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;

  operator uint32_t() const { return VK_MAKE_VERSION(major, minor, patch); }
};
} // namespace vk
