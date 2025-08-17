#pragma once

#include "engine/vulkan/extensions/swapchain.hpp"
class Pipeline {
protected:
  vk::raii::Pipeline pipeline;

public:
  Pipeline(vk::raii::Pipeline &&pipeline) : pipeline(std::move(pipeline)) {}
  auto operator*() -> vk::raii::Pipeline & { return pipeline; }

  auto operator*() const -> const vk::raii::Pipeline & { return pipeline; }

  operator vk::raii::Pipeline &() { return pipeline; }
  operator const vk::Pipeline &() const { return *pipeline; }
};

namespace pipelines {

class GreedyVoxel : public Pipeline {
public:
  static auto
  create(const vk::raii::Device &device,
         const engine::vulkan::SwapchainConfig &swapchainConfig,
         const std::span<vk::PipelineShaderStageCreateInfo> shaderStages)
      -> std::expected<GreedyVoxel, std::string>;
};
} // namespace pipelines
