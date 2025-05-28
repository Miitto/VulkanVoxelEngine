module;
#include "log.h"

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;

import :info.framebufferCreate;

namespace vk {
std::optional<Framebuffer>
Framebuffer::create(Device &device, vk::info::FramebufferCreate info) {

  VkFramebuffer framebuffer;
  if (vkCreateFramebuffer(*device, &info, nullptr, &framebuffer) !=
      VK_SUCCESS) {
    LOG_ERR("Failed to create framebuffer");
    return std::nullopt;
  }

  return Framebuffer(framebuffer, device);
}
} // namespace vk
