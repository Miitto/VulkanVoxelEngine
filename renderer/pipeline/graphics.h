#pragma once

#include "pipeline/pipeline.h"
#include "vulkan/vulkan.h"

class GraphicsPipeline : public Pipeline {
  GraphicsPipeline() = delete;
  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

public:
  GraphicsPipeline(VkPipeline pipeline, Device &device)
      : Pipeline(pipeline, device) {}

  GraphicsPipeline(GraphicsPipeline &&other) noexcept
      : Pipeline(std::move(other)) {}

  static std::optional<GraphicsPipeline>
  create(Device &device, VkGraphicsPipelineCreateInfo createInfo);

  VkPipelineBindPoint bindPoint() const override {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }
};
