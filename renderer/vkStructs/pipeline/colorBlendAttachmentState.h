#pragma once

#include "vulkan/vulkan.h"

class PipelineColorBlendAttachmentStateBuilder {
  VkPipelineColorBlendAttachmentState createInfo;

public:
  PipelineColorBlendAttachmentStateBuilder() {
    createInfo.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    createInfo.blendEnable = VK_FALSE;
    createInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    createInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    createInfo.colorBlendOp = VK_BLEND_OP_ADD;
    createInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    createInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    createInfo.alphaBlendOp = VK_BLEND_OP_ADD;
  }

  VkPipelineColorBlendAttachmentState build() { return createInfo; }
};
