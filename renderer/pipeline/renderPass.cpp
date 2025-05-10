#include "pipeline/renderPass.h"
#include <optional>

std::optional<RenderPass> RenderPass::create(Device &device,
                                             VkRenderPassCreateInfo info) {

  VkRenderPass renderPass;
  if (vkCreateRenderPass(*device, &info, nullptr, &renderPass) != VK_SUCCESS) {
    return std::nullopt;
  }

  RenderPass pass(device.ref(), renderPass);

  return std::move(pass);
}

RenderPass::~RenderPass() {
  if (renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(**device, renderPass, nullptr);
  }
}
