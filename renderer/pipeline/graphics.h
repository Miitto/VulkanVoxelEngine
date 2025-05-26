#pragma once

#include "pipeline/pipeline.h"
#include "structs/info/pipeline/graphicsPipelineCreate.h"

namespace vk {
class GraphicsPipeline : public Pipeline {
  GraphicsPipeline() = delete;
  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

public:
  GraphicsPipeline(VkPipeline pipeline, Device &device, PipelineLayout &layout)
      : Pipeline(pipeline, device, layout) {}

  GraphicsPipeline(GraphicsPipeline &&other) noexcept
      : Pipeline(std::move(other)) {}

  static std::optional<GraphicsPipeline>
  create(Device &device, vk::info::GraphicsPipelineCreate createInfo);

  VkPipelineBindPoint bindPoint() const override {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }
};
} // namespace vk
