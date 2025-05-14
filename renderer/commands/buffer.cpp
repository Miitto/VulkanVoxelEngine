#include "buffer.h"
#include "vkStructs/commands/bufferBeginInfo.h"
#include <optional>

using Encoder = CommandBuffer::Encoder;

Encoder CommandBuffer::begin() {
  CommandBufferBeginInfoBuilder beginInfo;

  begin(beginInfo.build());

  return **this->encoder;
}

Encoder CommandBuffer::begin(VkCommandBufferBeginInfo beginInfo) {
  if (encoder->has_value()) {
    return encoder->value();
  }

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  CommandBuffer::Encoder enc(*this);

  *encoder = std::move(enc);

  return **encoder;
}

Encoder::RenderPass Encoder::beginRenderPass(const VkRenderPassBeginInfo info,
                                             const VkSubpassContents contents) {
  vkCmdBeginRenderPass(**commandBuffer, &info, contents);

  CommandBuffer::Encoder::RenderPass renderPass(*this);

  *this->activeRenderPass = std::move(renderPass);

  return **activeRenderPass;
}

void Encoder::RenderPass::bindPipeline(const Pipeline &pipeline) {
  return bindPipeline(pipeline.bindPoint(), *pipeline);
}

void Encoder::RenderPass::bindPipeline(const VkPipelineBindPoint bindPoint,
                                       const VkPipeline &pipeline) {
  vkCmdBindPipeline(**encoder->commandBuffer, bindPoint, pipeline);
}

void Encoder::RenderPass::end() {
  if (!encoder->activeRenderPass->has_value()) {
    return;
  }
  encoder->activeRenderPass->reset();
  vkCmdEndRenderPass(**encoder->commandBuffer);
}

void Encoder::RenderPass::setViewport(const VkViewport &viewport) {
  vkCmdSetViewport(**encoder->commandBuffer, 0, 1, &viewport);
}

void Encoder::RenderPass::setScissor(const VkRect2D &scissor) {
  vkCmdSetScissor(**encoder->commandBuffer, 0, 1, &scissor);
}

void Encoder::RenderPass::draw(uint32_t vertexCount, uint32_t instanceCount,
                               uint32_t firstVertex, uint32_t firstInstance) {
  vkCmdDraw(**encoder->commandBuffer, vertexCount, instanceCount, firstVertex,
            firstInstance);
}

VkResult Encoder::end() {
  if (!commandBuffer->encoder->has_value()) {
    return VK_SUCCESS;
  }
  commandBuffer->encoder->reset();
  return vkEndCommandBuffer(**commandBuffer);
}
