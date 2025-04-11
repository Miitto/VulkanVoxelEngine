#pragma once
#include "instance.h"
#include "window.h"
#include <optional>
#include <utility>
#include <vulkan/vulkan.h>

class SurfacePtr {
  Instance instance;
  VkSurfaceKHR surface;

public:
  SurfacePtr(Instance instance, VkSurfaceKHR surface)
      : instance(instance), surface(surface) {}
  ~SurfacePtr() {
    if (surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(*instance, surface, nullptr);
    }
  }

  SurfacePtr(const SurfacePtr &) = delete;
  SurfacePtr &operator=(const SurfacePtr &) = delete;

  SurfacePtr(SurfacePtr &&o) noexcept
      : instance(o.instance), surface(std::move(o.surface)) {
    o.surface = VK_NULL_HANDLE;
  }

  SurfacePtr &operator=(SurfacePtr &&o) noexcept {
    surface = std::move(o.surface);
    o.surface = VK_NULL_HANDLE;
    return *this;
  }

  VkSurfaceKHR &operator*() { return surface; }
};

class Surface {
  std::shared_ptr<SurfacePtr> surface;

  Surface(Instance instance, VkSurfaceKHR surface)
      : surface(std::make_shared<SurfacePtr>(SurfacePtr(instance, surface))) {}

public:
  VkSurfaceKHR operator*() { return **surface; }

  static std::optional<Surface> create(Instance &instance, Window &window);
};
