#pragma once

#include "pipeline/layout.h"
#include "vulkan/vulkan_core.h"
#include <vector>

namespace vk {
namespace info {
class GraphicsPipelineCreate : public VkGraphicsPipelineCreateInfo {
  PipelineLayout::Ref m_layout;

public:
  GraphicsPipelineCreate()
      : VkGraphicsPipelineCreateInfo{.sType =
                                         VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                     .pNext = nullptr,
                                     .flags = 0,
                                     .stageCount = 0,
                                     .pStages = nullptr,
                                     .pVertexInputState = nullptr,
                                     .pInputAssemblyState = nullptr,
                                     .pTessellationState = nullptr,
                                     .pViewportState = nullptr,
                                     .pRasterizationState = nullptr,
                                     .pMultisampleState = nullptr,
                                     .pDepthStencilState = nullptr,
                                     .pColorBlendState = nullptr,
                                     .pDynamicState = nullptr,
                                     .layout = VK_NULL_HANDLE,
                                     .renderPass = VK_NULL_HANDLE,
                                     .subpass = 0,
                                     .basePipelineHandle = VK_NULL_HANDLE,
                                     .basePipelineIndex = -1},
        m_layout(nullptr) {}

  GraphicsPipelineCreate(
      PipelineLayout &pipelineLayout, VkRenderPass renderPass,
      std::vector<VkPipelineShaderStageCreateInfo> &stages,
      VkPipelineVertexInputStateCreateInfo vertexInputState,
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyState,
      VkPipelineViewportStateCreateInfo viewportState,
      VkPipelineRasterizationStateCreateInfo rasterizationState,
      VkPipelineMultisampleStateCreateInfo multisampleState,
      VkPipelineColorBlendStateCreateInfo colorBlendState,
      VkPipelineDynamicStateCreateInfo dynamicState, uint32_t subpassIndex = 0)
      : GraphicsPipelineCreate(
            pipelineLayout, renderPass, static_cast<uint32_t>(stages.size()),
            stages.data(), vertexInputState, inputAssemblyState, viewportState,
            rasterizationState, multisampleState, colorBlendState, dynamicState,
            subpassIndex) {}

  GraphicsPipelineCreate(
      PipelineLayout &pipelineLayout, VkRenderPass renderPass,
      uint32_t stageCount, VkPipelineShaderStageCreateInfo *stages,
      VkPipelineVertexInputStateCreateInfo vertexInputState,
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyState,
      VkPipelineViewportStateCreateInfo viewportState,
      VkPipelineRasterizationStateCreateInfo rasterizationState,
      VkPipelineMultisampleStateCreateInfo multisampleState,
      VkPipelineColorBlendStateCreateInfo colorBlendState,
      VkPipelineDynamicStateCreateInfo dynamicState, uint32_t subpassIndex = 0)
      : GraphicsPipelineCreate() {
    this->stageCount = stageCount;
    pStages = stages;
    pVertexInputState = &vertexInputState;
    pInputAssemblyState = &inputAssemblyState;
    pViewportState = &viewportState;
    pRasterizationState = &rasterizationState;
    pMultisampleState = &multisampleState;
    pColorBlendState = &colorBlendState;
    pDynamicState = &dynamicState;
    layout = *pipelineLayout;
    this->renderPass = renderPass;
    subpass = subpassIndex;
    m_layout = pipelineLayout.ref();
  }

  PipelineLayout &getLayout() { return m_layout.value(); }
};
} // namespace info
} // namespace vk
