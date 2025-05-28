module;

#include <vulkan/vulkan_core.h>

export module vk:version;

export namespace vk {
struct Version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;

  operator uint32_t() const { return VK_MAKE_VERSION(major, minor, patch); }
};

}
