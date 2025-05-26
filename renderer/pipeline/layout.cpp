#include "pipeline/layout.h"
#include <iostream>

namespace vk {
std::optional<PipelineLayout>
PipelineLayout::create(Device &device, vk::info::PipelineLayoutCreate info) {
  VkPipelineLayout layout;
  if (vkCreatePipelineLayout(*device, &info, nullptr, &layout) != VK_SUCCESS) {
    std::cerr << "Failed to create pipeline layout" << std::endl;
    return std::nullopt;
  }

  PipelineLayout pipelineLayout(layout, device.ref());

  return pipelineLayout;
}

PipelineLayout::~PipelineLayout() {
  if (layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(**device, layout, nullptr);
  }
}
} // namespace vk
