#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class PiplineViewportStateCreateInfoBuilder {
  VkPipelineViewportStateCreateInfo createInfo;
  std::vector<VkViewport> viewports;
  std::vector<VkRect2D> scissors;

public:
  PiplineViewportStateCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.viewportCount = 0;
    createInfo.pViewports = nullptr;
    createInfo.scissorCount = 0;
    createInfo.pScissors = nullptr;
  }

  PiplineViewportStateCreateInfoBuilder &
  addViewport(const VkViewport &viewport) {
    viewports.push_back(viewport);

    return *this;
  }

  PiplineViewportStateCreateInfoBuilder &addScissor(const VkRect2D &scissor) {
    scissors.push_back(scissor);

    return *this;
  }

  VkPipelineViewportStateCreateInfo build() {
    createInfo.viewportCount = static_cast<uint32_t>(viewports.size());
    createInfo.pViewports = viewports.data();
    createInfo.scissorCount = static_cast<uint32_t>(scissors.size());
    createInfo.pScissors = scissors.data();

    return createInfo;
  }
};
