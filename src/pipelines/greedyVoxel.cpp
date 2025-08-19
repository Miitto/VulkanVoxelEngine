#include "pipelines.hpp"

#include "logger.hpp"
#include <engine/vulkan/extensions/pipeline.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <engine/vulkan/extensions/swapchain.hpp>

namespace pipelines {
auto GreedyVoxel::create(const vk::raii::Device &device,
                         const engine::vulkan::SwapchainConfig &swapchainConfig)
    -> std::expected<GreedyVoxel, std::string> {
  Logger::trace("Creating Graphics Pipeline");

  auto shader_res = engine::vulkan::Shader::create(device, "basic.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto shaderStages = shaderModule.vertFrag();

  engine::vulkan::DynamicStateInfo dynamicStateInfo(vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor);

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleStrip};

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasSlopeFactor = 1.0f,
      .lineWidth = 1.0f};

  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False,
      .minSampleShading = 1.0f};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  vk::PipelineLayoutCreateInfo layoutInfo{.setLayoutCount = 0,
                                          .pushConstantRangeCount = 0};

  auto pipelineLayout_res = device.createPipelineLayout(layoutInfo);
  if (!pipelineLayout_res) {
    Logger::error("Failed to create pipeline layout: {}",
                  vk::to_string(pipelineLayout_res.error()));
    return std::unexpected("Failed to create pipeline layout");
  }

  auto &pipelineLayout = pipelineLayout_res.value();

  vk::PipelineRenderingCreateInfo renderingCreateInfo{
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapchainConfig.format.format,
  };

  vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
      .pNext = &renderingCreateInfo,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &colorBlending,
      .pDynamicState = dynamicStateInfo,
      .layout = pipelineLayout,
      .renderPass = nullptr,
  };

  Logger::trace("Creating Graphics Pipeline");
  auto pipeline_res =
      device.createGraphicsPipeline(nullptr, pipelineCreateInfo);

  if (!pipeline_res) {
    Logger::error("Failed to create graphics pipeline: {}",
                  vk::to_string(pipeline_res.error()));
    return std::unexpected("Failed to create graphics pipeline");
  }
  Logger::trace("Graphics Pipeline created");

  return GreedyVoxel(
      {std::move(pipelineLayout), std::move(pipeline_res.value())});
}
} // namespace pipelines
