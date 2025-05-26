#pragma once

#include "device/device.h"
#include "layout.h"
#include "vulkan/vulkan_core.h"

namespace vk {
class Pipeline : public RawRefable<Pipeline, VkPipeline> {
protected:
  VkPipeline pipeline;
  Device::Ref device;
  PipelineLayout::Ref m_layout;

  Pipeline() = delete;
  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  Pipeline &operator=(Pipeline &&) = delete;

public:
  using Ref = RawRef<Pipeline, VkPipeline>;
  Pipeline(VkPipeline pipeline, Device &device, PipelineLayout &pipelineLayout)
      : RawRefable(), pipeline(pipeline), device(device.ref()),
        m_layout(pipelineLayout.ref()) {}
  virtual ~Pipeline() {
    if (pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(**device, pipeline, nullptr);
    }
  }

  Pipeline(Pipeline &&other) noexcept
      : RawRefable(std::move(other)), pipeline(other.pipeline),
        device(other.device), m_layout(other.m_layout) {
    other.pipeline = VK_NULL_HANDLE;
  }

  VkPipeline operator*() const { return pipeline; }
  operator VkPipeline() const { return pipeline; }

  virtual VkPipelineBindPoint bindPoint() const = 0;

  PipelineLayout &layout() { return m_layout.value(); }
};
} // namespace vk
