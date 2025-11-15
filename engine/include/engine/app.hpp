#pragma once

#include <chrono>
#include <expected>
#include <string>

#include "backends/imgui_impl_vulkan.h"
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

#include <vk_mem_alloc.hpp>

namespace engine {

class MoveGuard {
public:
  MoveGuard() = default;
  MoveGuard(const MoveGuard &) = delete;
  MoveGuard(MoveGuard &&o) noexcept { o._moved = true; }
  MoveGuard &operator=(const MoveGuard &) = delete;
  MoveGuard &operator=(MoveGuard &&o) noexcept {
    if (this != &o) {
      o._moved = true;
    }
    return *this;
  }
  ~MoveGuard() = default;

  [[nodiscard]] bool moved() const noexcept { return _moved; }

protected:
  bool _moved = false;
};

class App {
public:
  struct Queues {
    vkh::Queue graphics;
    vkh::Queue present;
  };

  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&) noexcept;
  App &operator=(App &&) noexcept;

  virtual ~App() {
    if (moveGuard.moved())
      return;

    ImGui_ImplVulkan_Shutdown();

    device.waitIdle();
    allocator.destroyImage(renderImage.image, renderImage.alloc);

    allocator.destroy();
  }

  void poll() const noexcept { glfwPollEvents(); }

  struct FrameInfo {
    uint32_t frameIndex;
    uint32_t imageIndex;
  };

  enum class TickResult : uint8_t { Success, Recoverable, Bail };

  std::expected<FrameInfo, TickResult> newFrame() noexcept;

  TickResult presentFrame(FrameInfo frameInfo,
                          std::span<vk::CommandBuffer> cmdBuffers) noexcept;

  virtual TickResult update(float deltaTime) noexcept = 0;
  virtual TickResult render() noexcept = 0;

  void drawImGui(vk::raii::CommandBuffer &cmdBuffer,
                 uint32_t imageIndex) const noexcept;

  [[nodiscard]]
  auto shouldClose() const noexcept -> bool {
    return core.getWindow().shouldClose();
  }

  virtual void onWindowResize(Dimensions dim) noexcept = 0;

  auto recreateSwapchain() noexcept -> std::expected<void, std::string>;

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
  MoveGuard moveGuard;

  Input _input;
  engine::rendering::Core core;

  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;

  vma::Allocator allocator;

  Queues queues;

  vkh::Swapchain swapchain;
  vkh::AllocatedImage renderImage;

  vk::raii::CommandPool commandPool;

  std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects;

  uint32_t currentFrame = 0;

  struct OldSwapchain {
    vkh::Swapchain swapchain;
    uint32_t frameIndex;
  };

  std::optional<OldSwapchain> oldSwapchain = std::nullopt;

  ImGuiVkObjects imguiObjects;

  App(engine::rendering::Core &&core, vk::raii::PhysicalDevice &&physicalDevice,
      vk::raii::Device &&device, vma::Allocator allocator, Queues &&queues,
      vkh::Swapchain &&swapchain, vkh::AllocatedImage &&renderImage,
      vk::raii::CommandPool &&commandPool,
      std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> &&syncObjects,
      ImGuiVkObjects &&imGuiObjects) noexcept
      : core(std::move(core)), physicalDevice(std::move(physicalDevice)),
        device(std::move(device)), allocator(allocator),
        queues(std::move(queues)), swapchain(std::move(swapchain)),
        renderImage(std::move(renderImage)),
        commandPool(std::move(commandPool)),
        syncObjects(std::move(syncObjects)),
        imguiObjects(std::move(imGuiObjects)) {
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
    switch (app.update(deltaTime)) {
    case App::TickResult::Success:
      break;
    case App::TickResult::Recoverable:
      goto next;
    case App::TickResult::Bail:
      goto end;
    }

    switch (app.render()) {
    case App::TickResult::Success:
    case App::TickResult::Recoverable:
      break;
    case App::TickResult::Bail:
      goto end;
    }

  next:
    app.endFrame();

    lastFrame = now;
  }
end:
}
} // namespace engine
