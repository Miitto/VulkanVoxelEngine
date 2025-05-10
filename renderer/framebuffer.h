#pragma once

#include "device/device.h"
#include "vulkan/vulkan.h"

class Framebuffer {
  VkFramebuffer framebuffer;
  Device::Ref device;

  Framebuffer() = delete;
  Framebuffer(const Framebuffer &) = delete;
  Framebuffer &operator=(const Framebuffer &) = delete;
  Framebuffer &operator=(Framebuffer &&) = delete;

public:
  Framebuffer(VkFramebuffer framebuffer, Device &device)
      : framebuffer(framebuffer), device(device.ref()) {}

  Framebuffer(Framebuffer &&other) noexcept
      : framebuffer(other.framebuffer), device(other.device) {
    other.framebuffer = VK_NULL_HANDLE;
  }
  ~Framebuffer() {
    if (framebuffer != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(**device, framebuffer, nullptr);
    }
  }

  VkFramebuffer operator*() { return framebuffer; }

  static std::optional<Framebuffer> create(Device &device,
                                           VkFramebufferCreateInfo info);
};
