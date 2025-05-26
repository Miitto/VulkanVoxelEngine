#pragma once

#include "buffers/vertex.h"
#include "log.h"
#include "pipeline/layout.h"
#include "pipeline/pipeline.h"
#include "structs/info/bufferCopy.h"
#include "structs/info/commands/bufferBegin.h"
#include "vulkan/vulkan_core.h"
#include <assert.h>
#include <cstdint>
#include <span>

namespace vk {
class CommandBuffer {
  VkCommandBuffer commandBuffer;

public:
  class Encoder : public Refable<Encoder> {
    CommandBuffer *commandBuffer;

  public:
    Encoder(CommandBuffer &commandBuffer)
        : Refable(), commandBuffer(&commandBuffer), activeRenderPass(nullptr) {}

    Encoder(const Encoder &) = delete;
    Encoder &operator=(const Encoder &) = delete;
    Encoder(Encoder &&o)
        : Refable(std::move(o)), commandBuffer(o.commandBuffer),
          activeRenderPass(o.activeRenderPass) {}

    operator bool() const { return commandBuffer != nullptr; }

    class RenderPass : public Refable<RenderPass> {
      Encoder *encoder;
      RenderPass() = delete;
      std::optional<Pipeline::Ref> m_pipeline;

    public:
      RenderPass(Encoder &encoder) : Refable(), encoder(&encoder) {}
      RenderPass(const RenderPass &) = delete;
      RenderPass &operator=(const RenderPass &) = delete;
      RenderPass(RenderPass &&o) : Refable(std::move(o)), encoder(o.encoder) {
        o.encoder = nullptr;
      }
      operator bool() const { return encoder != nullptr && !!*encoder; }

      void bindPipeline(const vk::Pipeline &pipeline);

      void setViewport(const VkViewport &viewport);
      void setScissor(const VkRect2D &scissor);

      void bindVertexBuffer(uint32_t binding, VertexBuffer &buffer,
                            VkDeviceSize bufferOffset = 0);

      void bindVertexBuffers(uint32_t binding,
                             const std::span<VertexBuffer> &buffers,
                             const std::span<VkDeviceSize> &offsets);

      void bindIndexBuffer(
          IndexBuffer &buffer, VkDeviceSize offset = 0,
          VkIndexType indexType = VkIndexType::VK_INDEX_TYPE_UINT32);

      void bindDescriptorSet(const DescriptorSet &set,
                             const std::span<uint32_t> dynamicOffsets = {});
      void bindDescriptorSets(const std::span<DescriptorSet> &sets,
                              uint32_t firstSet = 0,
                              const std::span<uint32_t> dynamicOffsets = {});

      void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                uint32_t firstVertex = 0, uint32_t firstInstance = 0);

      void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                       uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                       uint32_t firstInstance = 0);

      void end();
      ~RenderPass();
    };

    Reference<RenderPass> activeRenderPass;

    RenderPass beginRenderPass(
        const VkRenderPassBeginInfo info,
        const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    void copyBuffer(Buffer &src, Buffer &dst, const VkBufferCopy &region);
    void copyBuffer(Buffer &src, Buffer &dst,
                    const std::span<vk::BufferCopy> &regions);

    struct TemporaryStaging {
      Buffer buf;
      DeviceMemory memory;
    };

    template <typename T>
    std::optional<CommandBuffer::Encoder::TemporaryStaging>
    writeBufferWithStaging(std::span<T> &data, Buffer &dst,
                           VkDeviceSize offset = 0) {
      if (!dst.canCopyTo()) {
        LOG_ERR("Buffer is not a transfer destination");
        return std::nullopt;
      }

      VkDeviceSize size = data.size() * sizeof(T);
      assert(size > 0);
      assert(dst.size() >= size + offset);

      auto &device = *dst.getDevice();

      auto stagingBufInfo =
          vk::info::BufferCreate(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
      auto stagingBuf_opt = device.createBuffer(stagingBufInfo);
      if (!stagingBuf_opt.has_value()) {
        LOG_ERR("Failed to create staging buffer");
        return std::nullopt;
      }
      auto &stagingBuffer = stagingBuf_opt.value();

      auto stagingMemory_opt = device.allocateMemory(
          stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      if (!stagingMemory_opt.has_value()) {
        LOG_ERR("Failed to allocate staging buffer memory");
        return std::nullopt;
      }
      auto &stagingMemory = stagingMemory_opt.value();

      if (stagingBuffer.bind(stagingMemory) != VK_SUCCESS) {
        LOG_ERR("Failed to bind staging buffer memory");
        return std::nullopt;
      }

      {
        auto mapping_opt = stagingMemory.map();
        if (!mapping_opt.has_value()) {
          LOG_ERR("Failed to map staging buffer");
          return std::nullopt;
        }
        auto &mapping = mapping_opt.value();

        mapping.write(data);
      }

      copyBuffer(
          stagingBuffer, dst,
          VkBufferCopy{.srcOffset = 0, .dstOffset = offset, .size = size});

      Encoder::TemporaryStaging staging{
          .buf = std::move(stagingBuffer),
          .memory = std::move(stagingMemory),
      };

      return staging;
    }

    VkResult end();

    ~Encoder();
  };

private:
  Reference<Encoder> encoder;

  CommandBuffer() = delete;

public:
  CommandBuffer(VkCommandBuffer commandBuffer)
      : commandBuffer(commandBuffer), encoder(Reference<Encoder>(nullptr)) {}

  VkCommandBuffer &operator*() { return commandBuffer; }

  Encoder begin();
  Encoder begin(vk::info::CommandBufferBegin info);

  void reset(VkCommandBufferResetFlags flags = 0) {
    vkResetCommandBuffer(commandBuffer, flags);
  }
};
} // namespace vk
