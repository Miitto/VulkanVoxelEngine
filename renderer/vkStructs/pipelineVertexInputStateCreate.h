#pragma once

#include <vulkan/vulkan.h>

class PipelineVertexInputStateCreateInfoBuilder {
  VkPipelineVertexInputStateCreateInfo createInfo;

public:
  PipelineVertexInputStateCreateInfoBuilder() {
    createInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.vertexBindingDescriptionCount = 0;
    createInfo.pVertexBindingDescriptions = nullptr;
    createInfo.vertexAttributeDescriptionCount = 0;
    createInfo.pVertexAttributeDescriptions = nullptr;
  }

  VkPipelineVertexInputStateCreateInfo build() { return createInfo; }
};
