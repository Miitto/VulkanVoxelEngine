#pragma once

#include <vulkan/vulkan.h>

class PipelineInputAssemblyStateCreateInfoBuilder {
  VkPipelineInputAssemblyStateCreateInfo createInfo;

public:
  PipelineInputAssemblyStateCreateInfoBuilder() {
    createInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.primitiveRestartEnable = VK_FALSE;
  }

  PipelineInputAssemblyStateCreateInfoBuilder &
  setTopology(VkPrimitiveTopology topology) {
    createInfo.topology = topology;
    return *this;
  }

  PipelineInputAssemblyStateCreateInfoBuilder &
  setPrimitiveRestartEnable(VkBool32 enable) {
    createInfo.primitiveRestartEnable = enable;
    return *this;
  }

  VkPipelineInputAssemblyStateCreateInfo build() { return createInfo; }
};
