#pragma once

#include "camera.hpp"
#include "pipelines/pipelines.hpp"
#include <engine/app.hpp>

class App : public engine::App {
public:
  static auto create() noexcept -> std::expected<App, std::string>;

  void update(float deltaTime) noexcept override;
  void render() noexcept override;

  ~App() { device.waitIdle(); }

protected:
#define EG_APP_PARAMS                                                          \
  engine::rendering::Core &&core, vk::raii::PhysicalDevice &&physicalDevice,   \
      vk::raii::Device &&device, Queues &&queues,                              \
      engine::vulkan::SwapchainConfig &&swapchainConfig,                       \
      engine::vulkan::Swapchain &&swapchain,                                   \
      vk::raii::CommandPool &&commandPool,                                     \
      std::array<engine::SyncObjects, MAX_FRAMES_IN_FLIGHT> &&syncObjects

  struct CameraObjects {
    vk::raii::DescriptorPool pool;
    PerspectiveCamera::Buffers buffers;
    PerspectiveCamera camera;
  };

  App(EG_APP_PARAMS,
      std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers,
      CameraObjects camera, pipelines::BasicVertex greedyPipeline,
      vk::raii::DeviceMemory vertexMemory,
      vk::raii::Buffer vertexBuffer) noexcept
      : engine::App(std::move(core), std::move(physicalDevice),
                    std::move(device), std::move(queues),
                    std::move(swapchainConfig), std::move(swapchain),
                    std::move(commandPool), std::move(syncObjects)),
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
