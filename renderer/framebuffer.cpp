#include "framebuffer.h"
#include "log.h"

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
