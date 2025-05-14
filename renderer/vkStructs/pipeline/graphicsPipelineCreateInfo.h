#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class GraphicsPipelineCreateInfoBuilder {
  VkGraphicsPipelineCreateInfo createInfo;

public:
  GraphicsPipelineCreateInfoBuilder(
      VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
      std::vector<VkPipelineShaderStageCreateInfo> &stages,
      VkPipelineVertexInputStateCreateInfo vertexInputState,
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyState,
      VkPipelineViewportStateCreateInfo viewportState,
      VkPipelineRasterizationStateCreateInfo rasterizationState,
      VkPipelineMultisampleStateCreateInfo multisampleState,
      VkPipelineColorBlendStateCreateInfo colorBlendState,
      VkPipelineDynamicStateCreateInfo dynamicState, uint32_t subpassIndex = 0)
      : GraphicsPipelineCreateInfoBuilder(
            pipelineLayout, renderPass, static_cast<uint32_t>(stages.size()), stages.data(),
            vertexInputState, inputAssemblyState, viewportState,
            rasterizationState, multisampleState, colorBlendState, dynamicState,
            subpassIndex) {}

  GraphicsPipelineCreateInfoBuilder(
      VkPipelineLayout pipelineLayout, VkRenderPass renderPass,
      uint32_t stageCount, VkPipelineShaderStageCreateInfo *stages,
      VkPipelineVertexInputStateCreateInfo vertexInputState,
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyState,
      VkPipelineViewportStateCreateInfo viewportState,
      VkPipelineRasterizationStateCreateInfo rasterizationState,
      VkPipelineMultisampleStateCreateInfo multisampleState,
      VkPipelineColorBlendStateCreateInfo colorBlendState,
      VkPipelineDynamicStateCreateInfo dynamicState,
      uint32_t subpassIndex = 0) {
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stageCount = stageCount;
    createInfo.pStages = stages;
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pTessellationState = nullptr;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = nullptr;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = pipelineLayout;
    createInfo.renderPass = renderPass;
    createInfo.subpass = subpassIndex;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;
  }

  VkGraphicsPipelineCreateInfo build() { return createInfo; }
};
