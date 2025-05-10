#pragma once

#include "swapchain.h"
#include <vulkan/vulkan.h>

class Viewport {
  VkViewport viewport;

public:
  Viewport(float width, float height) {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
  }

  Viewport(VkExtent2D extent) { Viewport(extent.width, extent.height); }

  Viewport(Swapchain &swapChain) { Viewport(swapChain.getExtent()); }
};
