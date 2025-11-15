#pragma once

#include "vkh/swapchain.hpp"

class Pipeline {
protected:
  vk::raii::PipelineLayout layout;
  vk::raii::Pipeline pipeline;

public:
  Pipeline(vk::raii::PipelineLayout &&layout,
           vk::raii::Pipeline &&pipeline) noexcept
      : layout(std::move(layout)), pipeline(std::move(pipeline)) {}
  auto operator*() noexcept -> vk::raii::Pipeline & { return pipeline; }

  auto operator*() const noexcept -> const vk::raii::Pipeline & {
    return pipeline;
  }

  operator vk::raii::Pipeline &() noexcept { return pipeline; }
  operator const vk::Pipeline &() const noexcept { return *pipeline; }

  [[nodiscard]]
  auto getLayout() noexcept -> vk::raii::PipelineLayout & {
    return layout;
  }
  [[nodiscard]] auto getLayout() const noexcept
      -> const vk::raii::PipelineLayout & {
    return layout;
  }
};

namespace pipelines {

class GreedyVoxel : public Pipeline {
public:
  static auto create(const vk::raii::Device &device,
                     const vkh::SwapchainConfig &swapchainConfig) noexcept
      -> std::expected<GreedyVoxel, std::string>;
};

class BasicVertex : public Pipeline {
public:
  struct DescriptorLayouts {
    const vk::raii::DescriptorSetLayout &camera;
  };
  static auto create(const vk::raii::Device &device,
                     const vkh::SwapchainConfig &swapchainConfig,
                     const DescriptorLayouts &layouts) noexcept
      -> std::expected<BasicVertex, std::string>;
};

} // namespace pipelines
