#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class RenderPassBeginInfoBuilder {
  VkRenderPassBeginInfo createInfo;
  std::vector<VkClearValue> clearValues;

public:
  RenderPassBeginInfoBuilder(VkRenderPass renderPass, VkFramebuffer framebuffer,
                             VkRect2D renderArea) {
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    createInfo.pNext = nullptr;
    createInfo.renderPass = renderPass;
    createInfo.framebuffer = framebuffer;
    createInfo.renderArea = renderArea;
    createInfo.clearValueCount = 0;
    createInfo.pClearValues = nullptr;
  }

  RenderPassBeginInfoBuilder &addClearValue(const VkClearValue &clearValue) {
    clearValues.push_back(clearValue);
    return *this;
  }

  VkRenderPassBeginInfo build() {
    createInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    createInfo.pClearValues = clearValues.data();
    return createInfo;
  }
};
