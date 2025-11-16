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

    allocator.destroyBuffer(vertexBuffer.buffer, vertexBuffer.alloc);
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
      CameraObjects camera, pipelines::Mesh greedyPipeline,
      vkh::AllocatedBuffer vertexBuffer) noexcept
      : engine::App(std::move(core), std::move(physicalDevice),
                    std::move(device), allocator, std::move(queues),
                    std::move(swapchain), std::move(renderImage),
                    std::move(commandPool), std::move(syncObjects),
                    std::move(imGuiObjects)),
        commandBuffers(std::move(commandBuffers)), camera(std::move(camera)),
        pipeline(std::move(greedyPipeline)), vertexBuffer(vertexBuffer) {
    vk::BufferDeviceAddressInfo bufferAddressInfo{.buffer =
                                                      vertexBuffer.buffer};

    vertexBufferAddress = device.getBufferAddress(bufferAddressInfo);
  }

  std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers;

  CameraObjects camera;

  pipelines::Mesh pipeline;
  vkh::AllocatedBuffer vertexBuffer;
  vk::DeviceAddress vertexBufferAddress = 0;
};
