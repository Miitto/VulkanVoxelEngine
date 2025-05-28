module;

#include <GLFW/glfw3.h>
#include <array>
#include <optional>
#include <vector>

export module app:cls;

import :common;

import vk;

class MoveGuard {
  bool moved = false;

public:
  MoveGuard(const MoveGuard &) = delete;
  MoveGuard &operator=(const MoveGuard &) = delete;
  MoveGuard() = default;
  MoveGuard(MoveGuard &&o) noexcept : moved(o.moved) { o.moved = true; }

  [[nodiscard]] bool isMoved() const { return moved; }
};

export class App {
  MoveGuard moveGuard;

public:
  static std::optional<App> create();

  // Since we have move-only members, we also need to be move-only
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&o) = default;

  void poll() const { glfwPollEvents(); }

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

  ~App() {
    if (moveGuard.isMoved()) {
      return;
    }

    device.waitIdle();

    glfwTerminate();
  }
};
