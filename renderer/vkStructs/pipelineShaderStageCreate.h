#pragma once

#include "../shader.h"
#include <vulkan/vulkan.h>

class PipelineShaderStageCreateInfoBuilder {
  VkPipelineShaderStageCreateInfo createInfo;

public:
  PipelineShaderStageCreateInfoBuilder(Shader shader) {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    switch (shader.getStage()) {
    case EShaderStage::VERTEX: {
      createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      break;
    }
    case EShaderStage::FRAG: {
      createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
    }
    case EShaderStage::GEOM: {
      createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      break;
    }
    }
    createInfo.module = *shader.getModule();
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
