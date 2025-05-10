#include "pipeline/layout.h"

std::optional<PipelineLayout>
PipelineLayout::create(Device &device, VkPipelineLayoutCreateInfo info) {
  VkPipelineLayout layout;
  if (vkCreatePipelineLayout(*device, &info, nullptr, &layout) != VK_SUCCESS) {
    std::cerr << "Failed to create pipeline layout" << std::endl;
    return std::nullopt;
  }

  PipelineLayout pipelineLayout(layout, device.ref());

  return std::move(pipelineLayout);
}

PipelineLayout::~PipelineLayout() {
  if (layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(**device, layout, nullptr);
  }
}
