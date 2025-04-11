#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class PipelineDynamicStateCreateInfoBuilder {
  VkPipelineDynamicStateCreateInfo createInfo;
  std::vector<VkDynamicState> dynamicStates;

public:
  PipelineDynamicStateCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
  }

  PipelineDynamicStateCreateInfoBuilder &addDynamicState(VkDynamicState state) {
    dynamicStates.push_back(state);
    return *this;
  }

  VkPipelineDynamicStateCreateInfo build() {
    createInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    createInfo.pDynamicStates = dynamicStates.data();
    return createInfo;
  }
};
