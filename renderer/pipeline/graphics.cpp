#include "graphics.h"

std::optional<GraphicsPipeline>
GraphicsPipeline::create(Device &device,
                         VkGraphicsPipelineCreateInfo createInfo) {
  VkPipeline pipeline;
  auto result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1,
                                          &createInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  return GraphicsPipeline(pipeline, device);
}
