#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
namespace info {
class PipelineLayoutCreate : public VkPipelineLayoutCreateInfo {
public:
  PipelineLayoutCreate()
      : VkPipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr} {}
};
} // namespace info
} // namespace vk
