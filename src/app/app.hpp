#pragma once

#include <expected>
#include <string>

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

  struct SwapchainConfig {
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentMode;
    vk::Extent2D extent;
    uint32_t minImageCount;
    uint32_t imageCount;
  };

  struct Swapchain {
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;
  };

  struct SyncObjects {
    vk::raii::Semaphore presentCompleteSemaphore;
    vk::raii::Semaphore renderCompleteSemaphore;
    vk::raii::Fence drawingFence;
  };

private:
  std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window;
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface;

  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;
  Queues queues;
  SwapchainConfig swapchainConfig;
  Swapchain swapchain;

  vk::raii::Pipeline pipeline;
  vk::raii::CommandPool commandPool;

  std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects;

  void setDebugMessenger(vk::raii::DebugUtilsMessengerEXT &&dMessenger) {
    debugMessenger = std::move(dMessenger);
  }

  int currentFrame = 0;

  struct OldSwapchain {
    vk::raii::SwapchainKHR swapchain;
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
    return glfwWindowShouldClose(window.get());
  }

  auto recreateSwapchain() -> std::expected<void, std::string>;

  App(std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> &window,
      vk::raii::Context &context, vk::raii::Instance &instance,
      vk::raii::SurfaceKHR &surface, vk::raii::PhysicalDevice &physicalDevice,
      vk::raii::Device &device, Queues &queues,
      SwapchainConfig &swapchainConfig, Swapchain &swapchain,
      vk::raii::Pipeline &pipeline, vk::raii::CommandPool &commandPool,
      std::array<SyncObjects, 2> &syncObjects)
      : window(std::move(window)), context(std::move(context)),
        instance(std::move(instance)), surface(std::move(surface)),
        physicalDevice(std::move(physicalDevice)), device(std::move(device)),
        queues(std::move(queues)), swapchainConfig(swapchainConfig),
        swapchain(std::move(swapchain)), pipeline(std::move(pipeline)),
        commandPool(std::move(commandPool)),
        syncObjects(std::move(syncObjects)) {}

  void preRender(const vk::raii::CommandBuffer &commandBuffer) {
    Logger::trace("Pre render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.images[currentFrame],
        vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
        {}, vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput);
  }

  void postRender(const vk::raii::CommandBuffer &commandBuffer) {
    Logger::trace("Post render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.images[currentFrame],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe);
  }

  [[nodiscard]] auto getSwapchainConfig() const -> const SwapchainConfig & {
    return swapchainConfig;
  }

  [[nodiscard]] auto getSwapchain() const -> const Swapchain & {
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

  [[nodiscard]] auto getWindow() const -> const GLFWwindow * {
    return window.get();
  }

  [[nodiscard]] auto getSwapchainImage(size_t index) const
      -> const vk::Image & {
    return swapchain.images[index];
  }

  [[nodiscard]] auto getSwapchainImageView(size_t index) const
      -> const vk::raii::ImageView & {
    return swapchain.imageViews[index];
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
  }

  enum class SwapchainState : uint8_t { Ok, Suboptimal, OutOfDate };

  [[nodiscard]] auto getNextImage()
      -> std::expected<std::pair<uint32_t, SwapchainState>, std::string> {
    auto &sync = getSyncObjects();
    while (vk::Result::eTimeout ==
           device.waitForFences({*sync.drawingFence}, VK_TRUE, UINT64_MAX)) {
      // Wait for the queue to become idle
    }
    auto [result, index] = swapchain.swapchain.acquireNextImage(
        std::numeric_limits<uint64_t>::max(), sync.presentCompleteSemaphore,
        nullptr);

    device.resetFences({*sync.drawingFence});

    if (result != vk::Result::eSuccess &&
        result != vk::Result::eSuboptimalKHR) {
      return std::unexpected("Failed to acquire next image: " +
                             vk::to_string(result));
    }

    if (result == vk::Result::eSuboptimalKHR) {
      Logger::warn("Swapchain is suboptimal, consider recreating it.");
      return std::make_pair(index, SwapchainState::Suboptimal);
    }

    if (result == vk::Result::eErrorOutOfDateKHR) {
      Logger::warn("Swapchain is out of date, recreate it.");
      return std::make_pair(index, SwapchainState::OutOfDate);
    }

    return std::make_pair(index, SwapchainState::Ok);
  }
};
