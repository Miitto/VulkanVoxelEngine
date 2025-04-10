#pragma once
#include "instance.h"
#include "window.h"
#include <optional>
#include <utility>
#include <vulkan/vulkan.h>

class Surface {
  Instance &instance;
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  Surface(Instance &instance, VkSurfaceKHR surface)
      : instance(instance), surface(surface) {}

public:
  Surface(const Surface &) = delete;
  Surface &operator=(const Surface &) = delete;
  Surface(Surface &&o) noexcept : instance(o.instance), surface(o.surface) {
    o.surface = VK_NULL_HANDLE;
  }
  Surface &operator=(Surface &&o) noexcept {
    instance = std::move(o.instance);
    surface = o.surface;
    o.surface = VK_NULL_HANDLE;
    return *this;
  };
  VkSurfaceKHR operator*() { return surface; }

  static std::optional<Surface> create(Instance &instance, Window &window);
  ~Surface();
};
