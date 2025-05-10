#pragma once

#include "vulkan/vulkan.h"

class PipelineMultisampleStateCreateInfoBuilder {
  VkPipelineMultisampleStateCreateInfo createInfo;

public:
  PipelineMultisampleStateCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    createInfo.sampleShadingEnable = VK_FALSE;
    createInfo.minSampleShading = 1.0f;
    createInfo.pSampleMask = nullptr;
    createInfo.alphaToCoverageEnable = VK_FALSE;
    createInfo.alphaToOneEnable = VK_FALSE;
  }

  VkPipelineMultisampleStateCreateInfo build() { return createInfo; }
};
