#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class PipelineColorBlendStateCreateInfoBuilder {
  VkPipelineColorBlendStateCreateInfo createInfo;
  std::vector<VkPipelineColorBlendAttachmentState> attachments;

public:
  PipelineColorBlendStateCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.logicOpEnable = VK_FALSE;
    createInfo.logicOp = VK_LOGIC_OP_COPY;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments = nullptr;
    createInfo.blendConstants[0] = 0.0f;
    createInfo.blendConstants[1] = 0.0f;
    createInfo.blendConstants[2] = 0.0f;
    createInfo.blendConstants[3] = 0.0f;
  }

  PipelineColorBlendStateCreateInfoBuilder &
  addAttachment(VkPipelineColorBlendAttachmentState attachment) {
    attachments.push_back(attachment);
    return *this;
  }

  VkPipelineColorBlendStateCreateInfo build() {
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();

    return createInfo;
  }
};
