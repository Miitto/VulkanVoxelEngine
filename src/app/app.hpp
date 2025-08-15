#pragma once

#include <expected>
#include <string>

#include "engine/core.hpp"
#include "engine/vulkan/extensions/swapchain.hpp"
#include "logger.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <GLFW/glfw3.h>
#include <memory>

#include <engine/image.hpp>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class App {
public:
  struct Queue {
    uint32_t index;
    std::shared_ptr<vk::raii::Queue> queue;
  };

  struct Queues {
    Queue graphicsQueue;
    Queue presentQueue;
  };

  struct SyncObjects {
    vk::raii::Semaphore presentCompleteSemaphore;
    vk::raii::Semaphore renderCompleteSemaphore;
    vk::raii::Fence drawingFence;
  };

private:
  engine::rendering::Core core;

  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;
  Queues queues;
  engine::vulkan::SwapchainConfig swapchainConfig;
  engine::vulkan::Swapchain swapchain;

  vk::raii::Pipeline pipeline;
  vk::raii::CommandPool commandPool;

  std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects;

  int currentFrame = 0;

  struct OldSwapchain {
    engine::vulkan::Swapchain swapchain;
    int frameIndex;
  };

  std::optional<OldSwapchain> oldSwapchain = std::nullopt;

public:
  App(const App &) = delete;
  App(App &&) = default;

  static auto create() -> std::expected<App, std::string>;

  [[nodiscard]] auto frameIndex() const -> int { return currentFrame; }

  void poll() const { glfwPollEvents(); }

  [[nodiscard]]
  auto shouldClose() const -> bool {
    return glfwWindowShouldClose(core.getWindow().get());
  }

  auto recreateSwapchain() -> std::expected<void, std::string>;

  App(engine::rendering::Core &core, vk::raii::PhysicalDevice &physicalDevice,
      vk::raii::Device &device, Queues &queues,
      engine::vulkan::SwapchainConfig &swapchainConfig,
      engine::vulkan::Swapchain &swapchain, vk::raii::Pipeline &pipeline,
      vk::raii::CommandPool &commandPool,
      std::array<SyncObjects, 2> &syncObjects)
      : core(std::move(core)), physicalDevice(std::move(physicalDevice)),
        device(std::move(device)), queues(std::move(queues)),
        swapchainConfig(swapchainConfig), swapchain(std::move(swapchain)),
        pipeline(std::move(pipeline)), commandPool(std::move(commandPool)),
        syncObjects(std::move(syncObjects)) {}

  void preRender(const vk::raii::CommandBuffer &commandBuffer,
                 const size_t index) {
    Logger::trace("Pre render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.nImage(index), vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal, {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput);
  }

  void postRender(const vk::raii::CommandBuffer &commandBuffer,
                  const size_t index) {
    Logger::trace("Post render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.nImage(index),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe);
  }

  [[nodiscard]] auto getSwapchainConfig() const
      -> const engine::vulkan::SwapchainConfig & {
    return swapchainConfig;
  }

  [[nodiscard]] auto getCore() const -> const engine::rendering::Core & {
    return core;
  }

  [[nodiscard]] auto getCore() -> engine::rendering::Core & { return core; }

  [[nodiscard]] auto getPhysicalDevice() const
      -> const vk::raii::PhysicalDevice & {
    return physicalDevice;
  }

  [[nodiscard]] auto getSwapchain() const -> const engine::vulkan::Swapchain & {
    return swapchain;
  }

  [[nodiscard]] auto getDevice() const -> const vk::raii::Device & {
    return device;
  }

  [[nodiscard]] auto getQueues() const -> const Queues & { return queues; }

  [[nodiscard]] auto getPipeline() const -> const vk::raii::Pipeline & {
    return pipeline;
  }

  [[nodiscard]] auto getSyncObjects() const -> const SyncObjects & {
    return syncObjects[currentFrame];
  }

  [[nodiscard]] auto getCurrentCommandPool() const
      -> const vk::raii::CommandPool & {
    return commandPool;
  }

  void endFrame() { currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; }

  [[nodiscard]] auto allocCmdBuffer(const vk::CommandBufferLevel level,
                                    const uint32_t count) const
      -> std::expected<std::vector<vk::raii::CommandBuffer>, std::string> {
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = *commandPool,
                                            .level = level,
                                            .commandBufferCount = count};

    auto cmdBuffers_res = device.allocateCommandBuffers(allocInfo);
    if (!cmdBuffers_res) {
      return std::unexpected("Failed to allocate command buffer");
    }

    auto &cmdBuffers = cmdBuffers_res.value();

    return std::move(cmdBuffers);
  }

  void checkSwapchain() {
    if (oldSwapchain.has_value()) {
      auto &old = oldSwapchain.value();

      if (device.waitForFences({*syncObjects[old.frameIndex].drawingFence},
                               VK_TRUE, 0) == vk::Result::eSuccess) {
        oldSwapchain = std::nullopt;
      }
    }

    auto size = core.getWindow().getNewSize();
    if (size.has_value()) {
      Logger::trace("Window resized, recreating swapchain");
      if (oldSwapchain.has_value()) {
        Logger::trace(
            "Recreating too fast, Waiting for old swapchain to be idle");
        device.waitIdle();
      }
      auto res = recreateSwapchain();
      if (!res) {
        Logger::error("Failed to recreate swapchain: {}", res.error());
      }
    }
  }

  auto getNextImage() {
    auto &sync = getSyncObjects();
    return swapchain.getNextImage(device, sync.drawingFence,
                                  sync.presentCompleteSemaphore);
  }
};
