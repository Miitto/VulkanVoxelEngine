#include "pipelines.hpp"

#include "logger.hpp"
#include <engine/util/macros.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>
#include <vkh/swapchain.hpp>

namespace pipelines {
auto Mesh::create(const vk::raii::Device &device, const vk::Format outFormat,
                  const DescriptorLayouts &layouts) noexcept
    -> std::expected<Mesh, std::string> {
  Logger::trace("Creating Graphics Pipeline");

  auto shader_res = vkh::Shader::create(device, "mesh.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto shaderStages = shaderModule.vertFrag();

  vk::PushConstantRange pushConstantRange{.stageFlags =
                                              vk::ShaderStageFlagBits::eVertex,
                                          .offset = 0,
                                          .size = sizeof(MeshPushConstants)};

  vk::PipelineLayoutCreateInfo layoutInfo{
      .setLayoutCount = 1,
      .pSetLayouts = &*layouts.camera,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange,
  };

  VK_MAKE(layout, device.createPipelineLayout(layoutInfo),
          "Failed to create pipeline layout");

  vkh::GraphicsPipelineConfig pipelineConfig = {
      .rendering = {.colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &outFormat},
      .shaders = shaderStages,
      .vertexInput = {},
      .inputAssembly = {.topology = vk::PrimitiveTopology::eTriangleStrip},
      .viewport = {.viewportCount = 1, .scissorCount = 1},
      .rasterizer = {.depthClampEnable = vk::False,
                     .rasterizerDiscardEnable = vk::False,
                     .polygonMode = vk::PolygonMode::eFill,
                     .cullMode = vk::CullModeFlagBits::eBack,
                     .frontFace = vk::FrontFace::eCounterClockwise,
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

  VK_MAKE(pipeline, device.createGraphicsPipeline(nullptr, cfg),
          "Failed to create graphics pipeline");
  Logger::trace("Graphics Pipeline created");

  return Mesh({std::move(layout), std::move(pipeline)});
}

} // namespace pipelines
