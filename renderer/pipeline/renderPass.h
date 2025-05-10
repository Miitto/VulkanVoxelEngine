#pragma once

#include "device/device.h"
#include "vulkan/vulkan.h"

class RenderPass {
  VkRenderPass renderPass;
  Device::Ref device;

  RenderPass(Device::Ref device, VkRenderPass renderPass)
      : renderPass(renderPass), device(device) {}

  RenderPass(const RenderPass &) = delete;
  RenderPass &operator=(const RenderPass &) = delete;
  RenderPass &operator=(RenderPass &&) = delete;

public:
  RenderPass(RenderPass &&other) noexcept
      : renderPass(other.renderPass), device(other.device) {
    other.renderPass = VK_NULL_HANDLE;
  }
  ~RenderPass();

  static std::optional<RenderPass> create(Device &device,
                                          VkRenderPassCreateInfo info);

  VkRenderPass operator*() { return renderPass; }
};
