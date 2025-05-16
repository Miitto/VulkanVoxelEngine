#include "buffer.h"
#include "buffers/vertex.h"
#include <cstdint>
#include <optional>

using Encoder = CommandBuffer::Encoder;

Encoder CommandBuffer::begin() {
  vk::info::CommandBufferBegin beginInfo;

  begin(beginInfo);

  return **this->encoder;
}

Encoder CommandBuffer::begin(vk::info::CommandBufferBegin beginInfo) {
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

void Encoder::RenderPass::bindVertexBuffer(uint32_t binding,
                                           VertexBuffer &buffer,
                                           VkDeviceSize offset) {
  vkCmdBindVertexBuffers(**encoder->commandBuffer, binding, 1, &*buffer,
                         &offset);
}

void Encoder::RenderPass::bindVertexBuffers(
    uint32_t binding, const std::span<VertexBuffer> &buffers,
    const std::span<VkDeviceSize> &offsets) {
  std::vector<VkBuffer> bufferHandles(buffers.size());
  for (size_t i = 0; i < buffers.size(); i++) {
    bufferHandles[i] = *buffers[i];
  }

  vkCmdBindVertexBuffers(**encoder->commandBuffer, binding,
                         static_cast<uint32_t>(buffers.size()),
                         bufferHandles.data(), offsets.data());
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
