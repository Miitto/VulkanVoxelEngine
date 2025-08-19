#pragma once

#include "engine/vulkan/extensions/swapchain.hpp"
class Pipeline {
protected:
  vk::raii::PipelineLayout layout;
  vk::raii::Pipeline pipeline;

public:
  Pipeline(vk::raii::PipelineLayout &&layout, vk::raii::Pipeline &&pipeline)
      : layout(std::move(layout)), pipeline(std::move(pipeline)) {}
  auto operator*() -> vk::raii::Pipeline & { return pipeline; }

  auto operator*() const -> const vk::raii::Pipeline & { return pipeline; }

  operator vk::raii::Pipeline &() { return pipeline; }
  operator const vk::Pipeline &() const { return *pipeline; }

  [[nodiscard]]
  auto getLayout() -> vk::raii::PipelineLayout & {
    return layout;
  }
  [[nodiscard]] auto getLayout() const -> const vk::raii::PipelineLayout & {
    return layout;
  }
};

namespace pipelines {

class GreedyVoxel : public Pipeline {
public:
  static auto create(const vk::raii::Device &device,
                     const engine::vulkan::SwapchainConfig &swapchainConfig)
      -> std::expected<GreedyVoxel, std::string>;
};

class BasicVertex : public Pipeline {
public:
  struct DescriptorLayouts {
    const vk::raii::DescriptorSetLayout &camera;
  };
  static auto create(const vk::raii::Device &device,
                     const engine::vulkan::SwapchainConfig &swapchainConfig,
                     const DescriptorLayouts &layouts)
      -> std::expected<BasicVertex, std::string>;
};

} // namespace pipelines
