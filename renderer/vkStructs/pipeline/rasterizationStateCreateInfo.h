#pragma once

#include "vulkan/vulkan.h"

class PipelineRasterizationStateCreateInfoBuilder {
  VkPipelineRasterizationStateCreateInfo createInfo;

public:
  PipelineRasterizationStateCreateInfoBuilder() {
    createInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.depthClampEnable = VK_FALSE;
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    createInfo.polygonMode = VK_POLYGON_MODE_FILL;
    createInfo.lineWidth = 1.0f;
    createInfo.cullMode = VK_CULL_MODE_NONE;
    createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    createInfo.depthBiasEnable = VK_FALSE;
    createInfo.depthBiasConstantFactor = 0.0f;
    createInfo.depthBiasClamp = 0.0f;
    createInfo.depthBiasSlopeFactor = 0.0f;
  }

  PipelineRasterizationStateCreateInfoBuilder &polygonMode(VkPolygonMode mode) {
    createInfo.polygonMode = mode;
    return *this;
  }

  PipelineRasterizationStateCreateInfoBuilder &lineWidth(float width) {
    createInfo.lineWidth = width;
    return *this;
  }

  PipelineRasterizationStateCreateInfoBuilder &cullMode(VkCullModeFlags mode,
                                                        VkFrontFace frontFace) {
    createInfo.cullMode = mode;
    createInfo.frontFace = frontFace;
    return *this;
  }

  VkPipelineRasterizationStateCreateInfo build() { return createInfo; }
};
