#include "buffer.h"
#include "buffers/vertex.h"
#include <cstdint>
#include "log.h"

using Encoder = CommandBuffer::Encoder;

Encoder CommandBuffer::begin() {
  vk::info::CommandBufferBegin beginInfo{};

  return begin(beginInfo);
}

Encoder CommandBuffer::begin(vk::info::CommandBufferBegin beginInfo) {
  if (encoder.has_value()) {
    return std::move(*encoder);
  }

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  CommandBuffer::Encoder enc(*this);

  encoder = enc.ref();

  return enc;
}

Encoder::RenderPass Encoder::beginRenderPass(const VkRenderPassBeginInfo info,
                                             const VkSubpassContents contents) {
  vkCmdBeginRenderPass(**commandBuffer, &info, contents);

  CommandBuffer::Encoder::RenderPass renderPass(*this);

  activeRenderPass = renderPass.ref();

  return renderPass;
}

void Encoder::RenderPass::bindPipeline(const Pipeline &pipeline) {
  if (!*this)
    return;
  return bindPipeline(pipeline.bindPoint(), *pipeline);
}

void Encoder::RenderPass::bindPipeline(const VkPipelineBindPoint bindPoint,
                                       const VkPipeline &pipeline) {
  if (!*this)
    return;
  vkCmdBindPipeline(**encoder->commandBuffer, bindPoint, pipeline);
}

void Encoder::RenderPass::end() {
  if (!*this)
    return;
  vkCmdEndRenderPass(**encoder->commandBuffer);
  encoder = nullptr;
}

Encoder::RenderPass::~RenderPass() {
  if (m_ref.is(this)) {
    end();
  }
}

void Encoder::RenderPass::setViewport(const VkViewport &viewport) {
  if (!*this)
    return;
  vkCmdSetViewport(**encoder->commandBuffer, 0, 1, &viewport);
}

void Encoder::RenderPass::setScissor(const VkRect2D &scissor) {
  if (!*this)
    return;
  vkCmdSetScissor(**encoder->commandBuffer, 0, 1, &scissor);
}

void Encoder::RenderPass::bindVertexBuffer(uint32_t binding,
                                           VertexBuffer &buffer,
                                           VkDeviceSize offset) {
  if (!*this)
    return;
  vkCmdBindVertexBuffers(**encoder->commandBuffer, binding, 1, &*buffer,
                         &offset);
}

void Encoder::RenderPass::bindVertexBuffers(
    uint32_t binding, const std::span<VertexBuffer> &buffers,
    const std::span<VkDeviceSize> &offsets) {
  if (!*this)
    return;
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
  if (!*this)
    return;
  vkCmdDraw(**encoder->commandBuffer, vertexCount, instanceCount, firstVertex,
            firstInstance);
}

void Encoder::copyBuffer(Buffer &src, Buffer &dst, const VkBufferCopy &region) {
  if (!*this)
    return;
  vkCmdCopyBuffer(**commandBuffer, *src, *dst, 1, &region);
}

void Encoder::copyBuffer(Buffer &src, Buffer &dst,
                         const std::span<vk::BufferCopy> &regions) {
  if (!*this)
    return;
  vkCmdCopyBuffer(**commandBuffer, *src, *dst,
                  static_cast<uint32_t>(regions.size()), regions.data());
}

VkResult Encoder::end() {
  if (!*this)
    return VK_SUCCESS;

  auto res = vkEndCommandBuffer(**commandBuffer);
  commandBuffer = nullptr;
  return res;
}

Encoder::~Encoder() {
  if (m_ref.is(this)) {
    end();
  }
}
