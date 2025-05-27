module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

namespace vk {
std::optional<GraphicsPipeline>
GraphicsPipeline::create(Device &device,
                         vk::info::GraphicsPipelineCreate createInfo) {
  VkPipeline pipeline;
  auto result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1,
                                          &createInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  return GraphicsPipeline(pipeline, device, createInfo.getLayout());
}
} // namespace vk
