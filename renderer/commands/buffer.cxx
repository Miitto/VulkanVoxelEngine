module;
#include "log.h"
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

namespace vk {
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
  m_pipeline = pipeline.ref();
  vkCmdBindPipeline(**encoder->commandBuffer, pipeline.bindPoint(), pipeline);
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

void Encoder::RenderPass::bindIndexBuffer(IndexBuffer &buffer,
                                          VkDeviceSize offset,
                                          VkIndexType indexType) {
  if (!*this)
    return;
  vkCmdBindIndexBuffer(**encoder->commandBuffer, *buffer, offset, indexType);
}

void Encoder::RenderPass::bindDescriptorSet(
    const DescriptorSet &set, const std::span<uint32_t> dynamicOffsets) {
  if (!*this)
    return;

  if (!m_pipeline.has_value()) {
    LOG_ERR("No pipeline bound to render pass, cannot bind descriptor set.");
    return;
  }

  if (!m_pipeline.value().has_value()) {
    LOG_ERR("Pipeline is not valid, cannot bind descriptor set.");
    return;
  }

  auto &pipeline = m_pipeline.value().value();

  VkDescriptorSet rawSet = set;

  vkCmdBindDescriptorSets(**encoder->commandBuffer, pipeline.bindPoint(),
                          pipeline.layout(), 0, 1, &rawSet,
                          dynamicOffsets.size(), dynamicOffsets.data());
}

void Encoder::RenderPass::bindDescriptorSets(
    const std::span<DescriptorSet> &sets, uint32_t firstSet,
    const std::span<uint32_t> dynamicOffsets) {
  if (!*this)
    return;

  if (!m_pipeline.has_value()) {
    LOG_ERR("No pipeline bound to render pass, cannot bind descriptor sets.");
    return;
  }

  if (!m_pipeline.value().has_value()) {
    LOG_ERR("Pipeline is not valid, cannot bind descriptor sets.");
    return;
  }

  auto &pipeline = m_pipeline.value().value();

  std::vector<VkDescriptorSet> rawSets(sets.size());
  for (size_t i = 0; i < sets.size(); i++) {
    rawSets[i] = *sets[i];
  }

  vkCmdBindDescriptorSets(**encoder->commandBuffer, pipeline.bindPoint(),
                          pipeline.layout(), firstSet,
                          static_cast<uint32_t>(sets.size()), rawSets.data(),
                          dynamicOffsets.size(), dynamicOffsets.data());
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

void Encoder::RenderPass::drawIndexed(uint32_t indexCount,
                                      uint32_t instanceCount,
                                      uint32_t firstIndex, int32_t vertexOffset,
                                      uint32_t firstInstance) {
  if (!*this)
    return;
  vkCmdDrawIndexed(**encoder->commandBuffer, indexCount, instanceCount,
                   firstIndex, vertexOffset, firstInstance);
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
} // namespace vk
