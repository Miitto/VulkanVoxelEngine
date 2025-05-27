module;
#include <optional>

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;

namespace vk {
std::optional<RenderPass> RenderPass::create(Device &device,
                                             vk::info::RenderPassCreate info) {

  VkRenderPass renderPass;
  if (vkCreateRenderPass(*device, &info, nullptr, &renderPass) != VK_SUCCESS) {
    return std::nullopt;
  }

  RenderPass pass(device.ref(), renderPass);

  return pass;
}

RenderPass::~RenderPass() {
  if (renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(**device, renderPass, nullptr);
  }
}
} // namespace vk
