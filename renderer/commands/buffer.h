#pragma once

#include "buffers/vertex.h"
#include "pipeline/pipeline.h"
#include "structs/info/bufferCopy.h"
#include "structs/info/commands/bufferBegin.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <span>

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

    public:
      RenderPass(Encoder &encoder) : Refable(), encoder(&encoder) {}
      RenderPass(const RenderPass &) = delete;
      RenderPass &operator=(const RenderPass &) = delete;
      RenderPass(RenderPass &&o) : Refable(std::move(o)), encoder(o.encoder) {
        o.encoder = nullptr;
      }
      operator bool() const { return encoder != nullptr && !!*encoder; }

      void bindPipeline(const VkPipelineBindPoint bindPoint,
                        const VkPipeline &pipeline);
      void bindPipeline(const Pipeline &pipeline);

      void setViewport(const VkViewport &viewport);
      void setScissor(const VkRect2D &scissor);

      void bindVertexBuffer(uint32_t binding, VertexBuffer &buffer,
                            VkDeviceSize bufferOffset = 0);

      void bindVertexBuffers(uint32_t binding,
                             const std::span<VertexBuffer> &buffers,
                             const std::span<VkDeviceSize> &offsets);

      void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                uint32_t firstVertex = 0, uint32_t firstInstance = 0);

      void end();
      ~RenderPass();
    };

    Ref<RenderPass> activeRenderPass;

    RenderPass beginRenderPass(
        const VkRenderPassBeginInfo info,
        const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    void copyBuffer(Buffer &src, Buffer &dst, const VkBufferCopy &region);
    void copyBuffer(Buffer &src, Buffer &dst,
                    const std::span<vk::BufferCopy> &regions);

    VkResult end();

    ~Encoder();
  };

private:
  Ref<Encoder> encoder;

  CommandBuffer() = delete;

public:
  CommandBuffer(VkCommandBuffer commandBuffer)
      : commandBuffer(commandBuffer), encoder(Ref<Encoder>(nullptr)) {}

  VkCommandBuffer &operator*() { return commandBuffer; }

  Encoder begin();
  Encoder begin(vk::info::CommandBufferBegin info);

  void reset(VkCommandBufferResetFlags flags = 0) {
    vkResetCommandBuffer(commandBuffer, flags);
  }
};
