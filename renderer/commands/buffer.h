#pragma once

#include "pipeline/pipeline.h"
#include "vulkan/vulkan.h"
#include <memory>

class CommandBuffer {
  VkCommandBuffer commandBuffer;

public:
  class Encoder {
    friend class CommandBuffer;
    CommandBuffer *commandBuffer;

  public:
    Encoder(CommandBuffer &commandBuffer)
        : commandBuffer(&commandBuffer),
          activeRenderPass(
              std::make_shared<std::optional<RenderPass>>(std::nullopt)) {}

    class RenderPass {
      friend class Encoder;
      Encoder *encoder;
      RenderPass() = delete;

    public:
      RenderPass(Encoder &encoder) : encoder(&encoder) {}

      void bindPipeline(const VkPipelineBindPoint bindPoint,
                        const VkPipeline &pipeline);
      void bindPipeline(const Pipeline &pipeline);

      void setViewport(const VkViewport &viewport);
      void setScissor(const VkRect2D &scissor);

      void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                uint32_t firstVertex = 0, uint32_t firstInstance = 0);

      void end();
    };
    friend class Encoder::RenderPass;
    std::shared_ptr<std::optional<Encoder::RenderPass>> activeRenderPass;

    RenderPass beginRenderPass(
        const VkRenderPassBeginInfo info,
        const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    VkResult end();
  };

private:
  std::shared_ptr<std::optional<CommandBuffer::Encoder>> encoder;

  CommandBuffer() = delete;

public:
  CommandBuffer(const CommandBuffer &o)
      : commandBuffer(o.commandBuffer), encoder(o.encoder) {};

  CommandBuffer &operator=(const CommandBuffer &o) {
    commandBuffer = o.commandBuffer;
    encoder = o.encoder;
    return *this;
  };

  CommandBuffer(VkCommandBuffer commandBuffer)
      : commandBuffer(commandBuffer),
        encoder(std::make_shared<std::optional<Encoder>>(std::nullopt)) {}

  VkCommandBuffer &operator*() { return commandBuffer; }

  Encoder begin();
  Encoder begin(VkCommandBufferBeginInfo info);

  void reset(VkCommandBufferResetFlags flags = 0) {
    vkResetCommandBuffer(commandBuffer, flags);
  }
};
