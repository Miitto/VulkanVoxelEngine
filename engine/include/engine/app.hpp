#pragma once

#include <chrono>
#include <expected>
#include <string>

#include "engine/core.hpp"
#include "vkh/swapchain.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <GLFW/glfw3.h>
#include <engine/util/macros.hpp>

#include <engine/image.hpp>

#include "defines.hpp"
#include "engine/input.hpp"
#include "engine/structs.hpp"
#include <vkh/structs.hpp>

namespace engine {

class App {
public:
  struct Queues {
    vkh::Queue graphics;
    vkh::Queue present;
  };

  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&) = default;
  App &operator=(App &&) = default;
  virtual ~App() = default;

  void poll() const noexcept { glfwPollEvents(); }

  virtual bool update(float deltaTime) noexcept = 0;
  virtual bool render() noexcept = 0;

  [[nodiscard]]
  auto shouldClose() const noexcept -> bool {
    return core.getWindow().shouldClose();
  }

  virtual void onWindowResize(Dimensions dim) noexcept = 0;

  auto recreateSwapchain() noexcept -> std::expected<void, std::string>;

  virtual void preRender(const vk::raii::CommandBuffer &commandBuffer,
                         const size_t index) noexcept;

  virtual void postRender(const vk::raii::CommandBuffer &commandBuffer,
                          const size_t index) noexcept;

  virtual void endFrame() noexcept;

  virtual void
  setupCmdBuffer(const vk::raii::CommandBuffer &cmdBuffer) const noexcept;

  [[nodiscard]] std::expected<std::vector<vk::raii::CommandBuffer>, std::string>
  allocCmdBuffer(const vk::CommandBufferLevel level,
                 const uint32_t count) const noexcept;

  void checkSwapchain() noexcept;

  struct SwapchainImageResult {
    uint32_t thisFrame;
    vkh::Swapchain::AcquireResult img;
  };

  std::expected<SwapchainImageResult, std::string> getNextImage() noexcept;

protected:
  Input _input;
  engine::rendering::Core core;

  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;
  Queues queues;

  vkh::SwapchainConfig swapchainConfig;
  vkh::Swapchain swapchain;

  vk::raii::CommandPool commandPool;

  std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects;

  uint32_t currentFrame = 0;

  struct OldSwapchain {
    vkh::Swapchain swapchain;
    uint32_t frameIndex;
  };

  std::optional<OldSwapchain> oldSwapchain = std::nullopt;

  App(engine::rendering::Core &&core, vk::raii::PhysicalDevice &&physicalDevice,
      vk::raii::Device &&device, Queues &&queues,
      vkh::SwapchainConfig &&swapchainConfig, vkh::Swapchain &&swapchain,
      vk::raii::CommandPool &&commandPool,
      std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> &&syncObjects) noexcept
      : core(std::move(core)), physicalDevice(std::move(physicalDevice)),
        device(std::move(device)), queues(std::move(queues)),
        swapchainConfig(swapchainConfig), swapchain(std::move(swapchain)),
        commandPool(std::move(commandPool)),
        syncObjects(std::move(syncObjects)) {
    auto &window = this->core.getWindow();
    window.setResizeCallback(
        [&](Dimensions dim) { this->onWindowResize(dim); });
  }
};

template <class T>
  requires std::is_base_of_v<App, T>
void run(T &app) {
  auto lastFrame = std::chrono::high_resolution_clock::now();
  while (!app.shouldClose()) {
    app.poll();
    auto now = std::chrono::high_resolution_clock::now();
    auto deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(
                         now - lastFrame)
                         .count();
    if (!app.update(deltaTime))
      return;
    if (!app.render())
      return;
    app.endFrame();

    lastFrame = now;
  }
}
} // namespace engine
