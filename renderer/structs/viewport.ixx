module;
#include <vulkan/vulkan_core.h>

export module vk:viewport;

import :swapchain;

export namespace vk {
class Viewport : public VkViewport {
public:
  Viewport(float width, float height)
      : VkViewport{.x = 0.0f,
                   .y = 0.0f,
                   .width = width,
                   .height = height,
                   .minDepth = 0.0f,
                   .maxDepth = 1.0f} {}

  Viewport(VkExtent2D extent) { Viewport(extent.width, extent.height); }

  Viewport(Swapchain &swapChain) { Viewport(swapChain.getExtent()); }
};
} // namespace vk
