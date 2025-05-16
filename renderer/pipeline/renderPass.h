#pragma once

#include "device/device.h"
#include "structs/info/renderPassCreate.h"
#include "vulkan/vulkan_core.h"

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
                                          vk::info::RenderPassCreate info);

  VkRenderPass operator*() { return renderPass; }
};
