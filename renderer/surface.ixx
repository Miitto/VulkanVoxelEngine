module;

#include <vector>
#include <optional>
#include <utility>
#include <vulkan/vulkan_core.h>

export module vk:surface;

import :instance;
import :device.physical;


export namespace vk {
class Surface {
  Instance::Ref instance;
  VkSurfaceKHR surface;

public:
  Surface(Instance &instance, VkSurfaceKHR surface)
      : instance(instance.ref()), surface(surface) {}
  ~Surface() {
    if (surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(**instance, surface, nullptr);
    }
  }

  Surface(const Surface &) = delete;
  Surface &operator=(const Surface &) = delete;
  Surface &operator=(Surface &&o) = delete;

  Surface(Surface &&o) noexcept
      : instance(o.instance), surface(std::move(o.surface)) {
    o.surface = VK_NULL_HANDLE;
  }

  static std::optional<Surface> create(Instance &instance, Window &window);

  VkSurfaceKHR &operator*() { return surface; }
};

class SurfaceAttributes {
public:
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;

  SurfaceAttributes(PhysicalDevice &device, Surface &surface);
};
} // namespace vk
