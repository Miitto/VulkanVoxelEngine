#pragma once

#include "device/device.h"
#include <vulkan/vulkan.h>

class PipelineLayout {
public:
  VkPipelineLayout layout;
  Device::Ref device;

  PipelineLayout(const PipelineLayout &) = delete;
  PipelineLayout &operator=(const PipelineLayout &) = delete;
  PipelineLayout &operator=(PipelineLayout &&) = delete;

  PipelineLayout(VkPipelineLayout layout, Device::Ref device)
      : layout(layout), device(device) {}

public:
  PipelineLayout(PipelineLayout &&other) noexcept
      : layout(other.layout), device(other.device) {
    other.layout = VK_NULL_HANDLE;
  }
  ~PipelineLayout();

  static std::optional<PipelineLayout> create(Device &device,
                                              VkPipelineLayoutCreateInfo info);

  VkPipelineLayout operator*() { return layout; }
};
