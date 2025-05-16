#pragma once

#include "shaders/shader.h"
#include <vulkan/vulkan_core.h>

namespace vk {
namespace info {
class PipelineShaderStageCreate : public VkPipelineShaderStageCreateInfo {

public:
  PipelineShaderStageCreate(Shader &shader)
      : VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = shader.getStage(),
            .module = *shader.getModule(),
            .pName = "main",
            .pSpecializationInfo = nullptr} {}
};
} // namespace info
} // namespace vk
