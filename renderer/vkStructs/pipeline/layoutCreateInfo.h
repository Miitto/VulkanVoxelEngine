#pragma once

#include "vulkan/vulkan.h"

class PipelineLayoutCreateInfoBuilder {
  VkPipelineLayoutCreateInfo createInfo;

public:
  PipelineLayoutCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;
  }

  VkPipelineLayoutCreateInfo build() { return createInfo; }
};
