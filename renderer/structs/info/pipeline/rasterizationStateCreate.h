#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
namespace info {

class PipelineRasterizationStateCreate
    : public VkPipelineRasterizationStateCreateInfo {

public:
  PipelineRasterizationStateCreate()
      : VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f} {}

  PipelineRasterizationStateCreate &setPolygonMode(VkPolygonMode mode) {
    polygonMode = mode;
    return *this;
  }

  PipelineRasterizationStateCreate &setLineWidth(float width) {
    lineWidth = width;
    return *this;
  }

  PipelineRasterizationStateCreate &setCullMode(VkCullModeFlags mode,
                                                VkFrontFace frontFace) {
    cullMode = mode;
    this->frontFace = frontFace;
    return *this;
  }
};
} // namespace info
} // namespace vk
