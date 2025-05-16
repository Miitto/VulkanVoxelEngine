#pragma once

#include <vulkan/vulkan_core.h>

namespace vk {
namespace info {

class PipelineInputAssemblyStateCreate
    : public VkPipelineInputAssemblyStateCreateInfo {

public:
  PipelineInputAssemblyStateCreate()
      : VkPipelineInputAssemblyStateCreateInfo{
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE} {}

  PipelineInputAssemblyStateCreate &setTopology(VkPrimitiveTopology topology) {
    this->topology = topology;
    return *this;
  }

  PipelineInputAssemblyStateCreate &setPrimitiveRestartEnable(VkBool32 enable) {
    primitiveRestartEnable = enable;
    return *this;
  }
};
} // namespace info
} // namespace vk
