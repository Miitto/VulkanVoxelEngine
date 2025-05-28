module;

#include "vulkan/vulkan_core.h"

export module vk:info.pipeline.multisampleStateCreate;

export namespace vk {
namespace info {
class PipelineMultisampleStateCreate
    : public VkPipelineMultisampleStateCreateInfo {
public:
  PipelineMultisampleStateCreate()
      : VkPipelineMultisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE} {}
};
} // namespace info
} // namespace vk
