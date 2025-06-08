#pragma once

#include <utility>
#include <vk/buffers.hpp>
#include <vk/commands/buffer.hpp>
#include <vk/commands/pool.hpp>
#include <vk/descriptors.hpp>
#include <vk/device/device.hpp>
#include <vk/device/memory.hpp>
#include <vk/framebuffer.hpp>
#include <vk/instance.hpp>
#include <vk/khr/surface.hpp>
#include <vk/khr/swapchain.hpp>
#include <vk/pipeline/graphics.hpp>
#include <vk/pipeline/layout.hpp>
#include <vk/pipeline/renderPass.hpp>
#include <vk/queue.hpp>
#include <vk/sync/fence.hpp>
#include <vk/sync/semaphore.hpp>
#include <vk/window.hpp>

#include <engine/core.hpp>

#include <array>
#include <optional>
#include <vector>

class MoveGuard {
  bool moved = false;

public:
  MoveGuard(const MoveGuard &) = delete;
  auto operator=(const MoveGuard &) -> MoveGuard & = delete;
  MoveGuard() = default;
  MoveGuard(MoveGuard &&o) noexcept : moved(o.moved) { o.moved = true; }

  [[nodiscard]] auto isMoved() const -> bool { return moved; }
};

const int MAX_FRAMES_IN_FLIGHT = 2;

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

class App {
  MoveGuard moveGuard;

public:
  static std::optional<App> create();

  // Since we have move-only members, we also need to be move-only
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&o) = default;

  void poll() const { engine::pollEvents(); }

  int currentFrame = 0;

  vk::Window window;
  vk::Instance instance;
  vk::khr::Surface surface;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::PresentQueue presentQueue;
  vk::khr::Swapchain swapchain;
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass renderPass;
  std::vector<vk::Framebuffer> framebuffers;
  vk::GraphicsPipeline pipeline;
  vk::CommandPool commandPool;
  std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames;
  VBufferParts vertexBuffer;
  vk::DescriptorPool descriptorPool;
  UObjects uniforms;

  App(vk::Window &window, vk::Instance &instance, vk::khr::Surface &surface,
      vk::Device &device, vk::Queue &graphicsQueue,
      vk::PresentQueue &presentQueue, vk::khr::Swapchain &swapchain,
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

  ~App() {
    if (moveGuard.isMoved()) {
      return;
    }

    device.waitIdle();
  }
};
