#pragma once

#include "device/device.h"
#include "vulkan/vulkan.h"

class Pipeline {
protected:
  VkPipeline pipeline;
  Device::Ref device;

  Pipeline() = delete;
  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;
  Pipeline &operator=(Pipeline &&) = delete;

public:
  Pipeline(VkPipeline pipeline, Device &device)
      : pipeline(pipeline), device(device.ref()) {}
  virtual ~Pipeline() {
    if (pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(**device, pipeline, nullptr);
    }
  }

  Pipeline(Pipeline &&other) noexcept
      : pipeline(other.pipeline), device(other.device) {
    other.pipeline = VK_NULL_HANDLE;
  }

  VkPipeline operator*() const { return pipeline; }

  virtual VkPipelineBindPoint bindPoint() const = 0;
};
