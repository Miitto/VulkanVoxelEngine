#include "framebuffer.h"

std::optional<Framebuffer> Framebuffer::create(Device &device,
                                               VkFramebufferCreateInfo info) {

  VkFramebuffer framebuffer;
  if (vkCreateFramebuffer(*device, &info, nullptr, &framebuffer) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create framebuffer" << std::endl;
    return std::nullopt;
  }

  return Framebuffer(framebuffer, device);
}
