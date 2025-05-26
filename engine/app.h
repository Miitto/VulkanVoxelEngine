#pragma once

#include "buffers/index.h"
#include "buffers/uniform.h"
#include "buffers/vertex.h"
#include "commands/buffer.h"
#include "commands/pool.h"
#include "device/device.h"
#include "device/memory.h"
#include "pipeline/graphics.h"
#include "pipeline/layout.h"
#include "swapchain.h"
#include "sync/fence.h"
#include "sync/semaphore.h"
#include <array>
#include <optional>

const int MAX_FRAMES_IN_FLIGHT = 2;

class MoveGuard {
  bool moved = false;

  MoveGuard(const MoveGuard &) = delete;
  MoveGuard &operator=(const MoveGuard &) = delete;

public:
  MoveGuard() = default;
  MoveGuard(MoveGuard &&o) noexcept : moved(o.moved) { o.moved = true; }

  bool isMoved() const { return moved; }
};

class App {
  MoveGuard moveGuard;

  void recordCommandBuffer(vk::CommandBuffer &commandBuffer,
                           uint32_t imageIndex);

public:
  static std::optional<App> create();
  void run();

  // Since we have move-only members, we also need to be move-only
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&o) = default;
  ~App();

  bool update();
  bool render();

  struct UObject {
    vk::UniformBuffer buffer;
    vk::MappingSegment bufferMapping;
    vk::DescriptorSet descriptorSet;
  };

  struct UObjects {
    vk::DeviceMemory memory;
    vk::Mapping mapping;
    std::array<UObject, MAX_FRAMES_IN_FLIGHT> objects;
  };

  struct Frame {
    vk::CommandBuffer commandBuffer;
    vk::Semaphore imageAvailable;
    vk::Semaphore renderFinished;
    vk::Fence inFlight;
  };

  struct VBufferParts {
    vk::VertexBuffer vertexBuffer;
    vk::IndexBuffer indexBuffer;
    vk::DeviceMemory memory;
  };

private:
  int currentFrame = 0;

  vk::Window window;
  vk::Instance instance;
  vk::Surface surface;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::PresentQueue presentQueue;
  vk::Swapchain swapchain;
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass renderPass;
  std::vector<vk::Framebuffer> framebuffers;
  vk::GraphicsPipeline pipeline;
  vk::CommandPool commandPool;
  std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames;
  VBufferParts vertexBuffer;
  vk::DescriptorPool descriptorPool;
  UObjects uniforms;

  App(vk::Window &window, vk::Instance &instance, vk::Surface &surface,
      vk::Device &device, vk::Queue graphicsQueue,
      vk::PresentQueue presentQueue, vk::Swapchain &swapchain,
      vk::PipelineLayout &pipelineLayout, vk::RenderPass &renderPass,
      std::vector<vk::Framebuffer> &framebuffers,
      vk::GraphicsPipeline &pipeline, vk::CommandPool &commandPool,
      std::array<Frame, MAX_FRAMES_IN_FLIGHT> &frames,
      VBufferParts &vertexBuffer, vk::DescriptorPool &descriptorPool,
      UObjects &uniforms)
      : window(std::move(window)), instance(std::move(instance)),
        surface(std::move(surface)), device(std::move(device)),
        graphicsQueue(graphicsQueue), presentQueue(presentQueue),
        swapchain(std::move(swapchain)),
        pipelineLayout(std::move(pipelineLayout)),
        renderPass(std::move(renderPass)),
        framebuffers(std::move(framebuffers)), pipeline(std::move(pipeline)),
        commandPool(std::move(commandPool)), frames(std::move(frames)),
        vertexBuffer(std::move(vertexBuffer)),
        descriptorPool(std::move(descriptorPool)),
        uniforms(std::move(uniforms)) {}
};
