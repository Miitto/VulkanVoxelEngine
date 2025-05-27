module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:structs.info.pipeline.layoutCreate;

import :descriptors;

export namespace vk {
namespace info {
class PipelineLayoutCreate : public VkPipelineLayoutCreateInfo {
  std::vector<DescriptorSetLayout::Ref> layouts;
  std::vector<VkDescriptorSetLayout> layoutHandles;

  void setupLayouts() {
    setLayoutCount = static_cast<uint32_t>(layouts.size());
    layoutHandles.resize(setLayoutCount);
    for (size_t i = 0; i < setLayoutCount; ++i) {
      layoutHandles[i] = *layouts[i];
    }
    pSetLayouts = layoutHandles.data();
  }

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

  PipelineLayoutCreate(VkPipelineLayoutCreateInfo &&other)
      : VkPipelineLayoutCreateInfo(other) {}

  PipelineLayoutCreate &addSetLayout(DescriptorSetLayout &layout) {
    layouts.push_back(layout.ref());
    setupLayouts();
    return *this;
  }
};
} // namespace info
} // namespace vk
