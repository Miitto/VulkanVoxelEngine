#pragma once

#include "../shader.h"
#include <vulkan/vulkan.h>

class PipelineShaderStageCreateInfoBuilder {
  VkPipelineShaderStageCreateInfo createInfo;

public:
  PipelineShaderStageCreateInfoBuilder(ShaderModule module) {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    switch (module.getStage()) {
    case VERTEX: {
      createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      break;
    }
    case FRAG: {
      createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
    }
    case GEOM: {
      createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      break;
    }
    case TESS: {
      createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      break;
    }
    }
    createInfo.module = *module;
    createInfo.pName = "main";
  }

  PipelineShaderStageCreateInfoBuilder &setPNext(void *pNext) {
    createInfo.pNext = pNext;
    return *this;
  }

  PipelineShaderStageCreateInfoBuilder &
  setFlags(VkPipelineShaderStageCreateFlags flags) {
    createInfo.flags = flags;
    return *this;
  }

  PipelineShaderStageCreateInfoBuilder &setPName(const char *pName) {
    createInfo.pName = pName;
    return *this;
  }

  VkPipelineShaderStageCreateInfo build() { return createInfo; }
};
