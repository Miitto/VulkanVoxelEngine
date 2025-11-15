#pragma once

#include "camera.hpp"
#include "pipelines/pipelines.hpp"
#include <engine/app.hpp>

class App : public engine::App {
public:
  static auto create() noexcept -> std::expected<App, std::string>;

  App() = delete;
  App(const App &) = delete;
  App(App &&) = default;

  TickResult update(float deltaTime) noexcept override;
  TickResult render() noexcept override;
  void draw(vk::raii::CommandBuffer &cmdBuffer);

  void onWindowResize(engine::Dimensions dim) noexcept override;

  ~App() override {
    if (moveGuard.moved())
      return;

    device.waitIdle();
  }

protected:
  struct CameraObjects {
    vk::raii::DescriptorPool pool;
    PerspectiveCamera::Buffers buffers;
    PerspectiveCamera camera;
  };

  App(engine::rendering::Core &&core, vk::raii::PhysicalDevice &&physicalDevice,
      vk::raii::Device &&device, vma::Allocator allocator, Queues &&queues,
      vkh::Swapchain &&swapchain, vkh::AllocatedImage &&renderImage,
      vk::raii::CommandPool &&commandPool,
      std::array<engine::SyncObjects, MAX_FRAMES_IN_FLIGHT> &&syncObjects,
      engine::ImGuiVkObjects &&imGuiObjects,
      std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers,
      CameraObjects camera, pipelines::BasicVertex greedyPipeline,
      vk::raii::DeviceMemory vertexMemory,
      vk::raii::Buffer vertexBuffer) noexcept
      : engine::App(std::move(core), std::move(physicalDevice),
                    std::move(device), allocator, std::move(queues),
                    std::move(swapchain), std::move(renderImage),
                    std::move(commandPool), std::move(syncObjects),
                    std::move(imGuiObjects)),
        commandBuffers(std::move(commandBuffers)), camera(std::move(camera)),
        pipeline(std::move(greedyPipeline)),
        vBufferMemory(std::move(vertexMemory)),
        vertexBuffer(std::move(vertexBuffer)) {}

  std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;

  CameraObjects camera;

  pipelines::BasicVertex pipeline;
  vk::raii::DeviceMemory vBufferMemory;
  vk::raii::Buffer vertexBuffer;
};
