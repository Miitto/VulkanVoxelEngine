module;

#include "refs/refable.h"
#include <vulkan/vulkan_core.h>
#include <optional>

export module vk:pipeline.layout;

import :device;
import :info.pipeline.layoutCreate;

export namespace vk {
class PipelineLayout : public RawRefable<PipelineLayout, VkPipelineLayout> {
public:
  using Ref = RawRef<PipelineLayout, VkPipelineLayout>;
  VkPipelineLayout layout;
  Device::Ref device;

  PipelineLayout(const PipelineLayout &) = delete;
  PipelineLayout &operator=(const PipelineLayout &) = delete;
  PipelineLayout &operator=(PipelineLayout &&) = delete;

  PipelineLayout(VkPipelineLayout layout, Device::Ref device)
      : RawRefable(), layout(layout), device(device) {}

public:
  PipelineLayout(PipelineLayout &&other) noexcept
      : RawRefable(std::move(other)), layout(other.layout),
        device(other.device) {
    other.layout = VK_NULL_HANDLE;
  }
  ~PipelineLayout();

  static std::optional<PipelineLayout>
  create(Device &device, vk::info::PipelineLayoutCreate info);

  VkPipelineLayout operator*() { return layout; }
  operator VkPipelineLayout() { return layout; }
};
} // namespace vk
