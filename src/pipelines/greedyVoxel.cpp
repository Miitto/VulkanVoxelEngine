#include "pipelines.hpp"

#include "logger.hpp"
#include <engine/util/macros.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>
#include <vkh/swapchain.hpp>

namespace pipelines {
auto GreedyVoxel::create(const vk::raii::Device &device,
                         const vkh::SwapchainConfig &swapchainConfig) noexcept
    -> std::expected<GreedyVoxel, std::string> {
  Logger::trace("Creating Graphics Pipeline");

  auto shader_res = vkh::Shader::create(device, "basic.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto shaderStages = shaderModule.vertFrag();

  vk::PipelineLayoutCreateInfo layoutInfo{.setLayoutCount = 0,
                                          .pushConstantRangeCount = 0};
  VK_MAKE(layout, device.createPipelineLayout(layoutInfo),
          "Failed to create pipeline layout");

  vkh::GraphicsPipelineConfig pipelineConfig = {
      .rendering = {.colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &swapchainConfig.format.format},
      .shaders = shaderStages,
      .vertexInput = {},
      .inputAssembly = {.topology = vk::PrimitiveTopology::eTriangleStrip},
      .viewport = {.viewportCount = 1, .scissorCount = 1},
      .rasterizer = {.depthClampEnable = vk::False,
                     .rasterizerDiscardEnable = vk::False,
                     .polygonMode = vk::PolygonMode::eFill,
                     .cullMode = vk::CullModeFlagBits::eBack,
                     .frontFace = vk::FrontFace::eClockwise,
                     .depthBiasEnable = vk::False,
                     .depthBiasSlopeFactor = 1.0f,
                     .lineWidth = 1.0f},
      .multisampling = {.rasterizationSamples = vk::SampleCountFlagBits::e1,
                        .sampleShadingEnable = vk::False,
                        .minSampleShading = 1.0f},
      .blendAttachments = {{.blendEnable = vk::False,
                            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                                              vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB |
                                              vk::ColorComponentFlagBits::eA}},
      .blending = {.logicOpEnable = vk::False},
      .dynamicState = {vk::DynamicState::eViewport, vk::DynamicState::eScissor},
      .layout = layout,
  };

  Logger::trace("Creating Graphics Pipeline");

  auto cfg = pipelineConfig.build();

  auto pipeline_res = device.createGraphicsPipeline(nullptr, cfg);

  if (!pipeline_res) {
    Logger::error("Failed to create graphics pipeline: {}",
                  vk::to_string(pipeline_res.error()));
    return std::unexpected("Failed to create graphics pipeline");
  }
  Logger::trace("Graphics Pipeline created");

  return GreedyVoxel({std::move(layout), std::move(pipeline_res.value())});
}
} // namespace pipelines
