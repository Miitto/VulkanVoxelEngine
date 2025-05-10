#pragma once

#include "commands/pool.h"
#include "device/device.h"
#include "framebuffer.h"
#include "pipeline/renderPass.h"
#include "queue.h"
#include "swapchain.h"
#include "sync/fence.h"
#include "sync/semaphore.h"
#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "instance.h"
#include "pipeline/graphics.h"
#include "pipeline/layout.h"
#include "surface.h"
#include "window.h"

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

  void recordCommandBuffer(CommandBuffer &commandBuffer, uint32_t imageIndex);

public:
  static std::optional<App> create();
  void run();

  // Since we have move-only members, we also need to be move-only
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&o) = default;
  ~App();

  void update();
  void render();

  struct SyncObjects {
    Semaphore imageAvailable;
    Semaphore renderFinished;
    Fence inFlight;
  };

private:
  Window window;
  Instance instance;
  Surface surface;
  Device device;
  Queue graphicsQueue;
  PresentQueue presentQueue;
  Swapchain swapchain;
  PipelineLayout pipelineLayout;
  RenderPass renderPass;
  std::vector<Framebuffer> framebuffers;
  GraphicsPipeline pipeline;
  CommandPool commandPool;
  CommandBuffer commandBuffer;
  SyncObjects syncObjects;

  App(Window &window, Instance &instance, Surface &surface, Device &device,
      Queue graphicsQueue, PresentQueue presentQueue, Swapchain &swapchain,
      PipelineLayout &pipelineLayout, RenderPass &renderPass,
      std::vector<Framebuffer> &framebuffers, GraphicsPipeline &pipeline,
      CommandPool &commandPool, CommandBuffer commandBuffer,
      SyncObjects &syncObjects)
      : window(std::move(window)), instance(std::move(instance)),
        surface(std::move(surface)), device(std::move(device)),
        graphicsQueue(graphicsQueue), presentQueue(presentQueue),
        swapchain(std::move(swapchain)),
        pipelineLayout(std::move(pipelineLayout)),
        renderPass(std::move(renderPass)),
        framebuffers(std::move(framebuffers)), pipeline(std::move(pipeline)),
        commandPool(std::move(commandPool)), commandBuffer(commandBuffer),
        syncObjects(std::move(syncObjects)) {}
};
