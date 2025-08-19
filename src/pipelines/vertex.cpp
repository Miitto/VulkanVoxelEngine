#include "pipelines.hpp"

#include "logger.hpp"
#include "vertex.hpp"
#include <engine/util/macros.hpp>
#include <engine/vulkan/extensions/pipeline.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <engine/vulkan/extensions/swapchain.hpp>

namespace pipelines {
auto BasicVertex::create(const vk::raii::Device &device,
                         const engine::vulkan::SwapchainConfig &swapchainConfig,
                         const DescriptorLayouts &layouts) noexcept
    -> std::expected<BasicVertex, std::string> {
  Logger::trace("Creating Graphics Pipeline");

  auto shader_res = engine::vulkan::Shader::create(device, "vertex.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto shaderStages = shaderModule.vertFrag();

  engine::vulkan::DynamicStateInfo dynamicStateInfo(vk::DynamicState::eViewport,
                                                    vk::DynamicState::eScissor);

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &bindingDescription,
      .vertexAttributeDescriptionCount = attributeDescriptions.size(),
      .pVertexAttributeDescriptions = attributeDescriptions.data(),
  };

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleStrip};

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eCounterClockwise,
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

  vk::PipelineLayoutCreateInfo layoutInfo{.setLayoutCount = 1,
                                          .pSetLayouts = &*layouts.camera,
                                          .pushConstantRangeCount = 0};

  VK_MAKE(pipelineLayout, device.createPipelineLayout(layoutInfo),
          "Failed to create pipeline layout");

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
  VK_MAKE(pipeline, device.createGraphicsPipeline(nullptr, pipelineCreateInfo),
          "Failed to create graphics pipeline");
  Logger::trace("Graphics Pipeline created");

  return BasicVertex({std::move(pipelineLayout), std::move(pipeline)});
}

} // namespace pipelines
