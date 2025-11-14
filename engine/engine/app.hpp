#pragma once

#include <expected>
#include <string>

#include "engine/core.hpp"
#include "engine/vulkan/extensions/swapchain.hpp"
#include "logger.hpp"
#include "vulkan/vulkan_raii.hpp"
#include <GLFW/glfw3.h>
#include <engine/util/macros.hpp>
#include <memory>

#include <engine/image.hpp>

#include "defines.hpp"
#include "engine/input.hpp"
#include "vulkan/physicalDeviceSelector.hpp"

namespace engine {
struct Queue {
  uint32_t index;
  std::shared_ptr<vk::raii::Queue> queue;
};

struct SyncObjects {
  vk::raii::Semaphore presentCompleteSemaphore;
  vk::raii::Semaphore renderCompleteSemaphore;
  vk::raii::Fence drawingFence;
};

namespace setup {
std::expected<vk::raii::PhysicalDevice, std::string>
selectPhysicalDevice(const vk::raii::Instance &instance,
                     std::function<std::optional<std::string>(
                         engine::vulkan::PhysicalDeviceSelector &)>);

struct CoreQueueFamilyIndices {
  uint32_t graphics;
  uint32_t present;

  [[nodiscard]] auto uniqueIndices() const noexcept -> std::vector<uint32_t> {
    if (graphics == present) {
      return {graphics};
    } else {
      return {graphics, present};
    }
  }
};

std::expected<CoreQueueFamilyIndices, std::string>
findCoreQueues(const vk::raii::PhysicalDevice &, const vk::raii::SurfaceKHR &);

struct QueueCreateInfo {
  uint32_t familyIndex;
  float priority = 1.0f;
};

const vk::StructureChain<vk::PhysicalDeviceFeatures2,
                         vk::PhysicalDeviceVulkan11Features,
                         vk::PhysicalDeviceVulkan13Features,
                         vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
    ENGINE_DEVICE_EXTENSIONS = {
        {},
        {.shaderDrawParameters = true},
        {.synchronization2 = true, .dynamicRendering = true},
        {.extendedDynamicState = true}};

template <typename... Ts>
std::expected<vk::raii::Device, std::string>
createLogicalDevice(const vk::raii::PhysicalDevice &physicalDevice,
                    std::span<QueueCreateInfo> indices,
                    const vk::StructureChain<Ts...> &chain,
                    std::span<const char *const> deviceExtensions) {
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
  queueCreateInfo.reserve(indices.size());
  for (const auto &index : indices) {
    std::array<float, 1> queuePriority = {index.priority};
    queueCreateInfo.push_back(
        vk::DeviceQueueCreateInfo{.queueFamilyIndex = index.familyIndex,
                                  .queueCount = 1,
                                  .pQueuePriorities = queuePriority.data()});
  }

  // Source - https://stackoverflow.com/a
  // Posted by Barry, modified by community. See post 'Timeline' for change
  // history Retrieved 2025-11-14, License - CC BY-SA 4.0
  using ChainStart = std::tuple_element_t<0, std::tuple<Ts...>>;

  auto deviceCreateInfo = vk::DeviceCreateInfo{
      .pNext = &chain.get<ChainStart>(),
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
      .pQueueCreateInfos = queueCreateInfo.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  VK_MAKE(device, physicalDevice.createDevice(deviceCreateInfo),
          "Failed to create logical device");
  return std::move(device);
}

std::expected<std::vector<Queue>, std::string>
retrieveQueues(const vk::raii::Device &device,
               const std::vector<uint32_t> &indices);

auto createSwapchain(
    const vk::raii::PhysicalDevice &physicalDevice,
    const vk::raii::Device &device, GLFWwindow *window,
    const vk::raii::SurfaceKHR &surface, const CoreQueueFamilyIndices &queues,
    std::optional<vk::raii::SwapchainKHR *> oldSwapchain) noexcept
    -> std::expected<
        std::tuple<engine::vulkan::SwapchainConfig, engine::vulkan::Swapchain>,
        std::string>;

auto createSyncObjects(const vk::raii::Device &device) noexcept
    -> std::expected<SyncObjects, std::string>;
} // namespace setup

class App {
public:
  struct Queues {
    Queue graphics;
    Queue present;
  };

  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&) = default;
  App &operator=(App &&) = default;

  void poll() const noexcept { glfwPollEvents(); }

  virtual void update(float deltaTime) noexcept = 0;
  virtual void render() noexcept = 0;

  [[nodiscard]]
  auto shouldClose() const noexcept -> bool {
    return core.getWindow().shouldClose();
  }

  auto recreateSwapchain() noexcept -> std::expected<void, std::string>;

  virtual void preRender(const vk::raii::CommandBuffer &commandBuffer,
                         const size_t index) noexcept {
    Logger::trace("Pre render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.nImage(index), vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal, {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eTopOfPipe,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput);
  }

  virtual void postRender(const vk::raii::CommandBuffer &commandBuffer,
                          const size_t index) noexcept {
    Logger::trace("Post render");
    engine::transitionImageLayout(
        commandBuffer, swapchain.nImage(index),
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe);
  }

  virtual void setupCmdBuffer(const vk::raii::CommandBuffer &cmdBuffer) {
    cmdBuffer.setViewport(
        0, vk::Viewport{
               .x = 0.0f,
               .y = 0.0f,
               .width = static_cast<float>(getSwapchainConfig().extent.width),
               .height = static_cast<float>(getSwapchainConfig().extent.height),
               .minDepth = 1.0f,
               .maxDepth = 0.0f});

    cmdBuffer.setScissor(0, vk::Rect2D{.offset = {.x = 0, .y = 0},
                                       .extent = getSwapchainConfig().extent});
  }

  [[nodiscard]] auto input() const noexcept -> const Input & { return _input; }
  [[nodiscard]] auto input() noexcept -> Input & { return _input; }

  [[nodiscard]] auto getSwapchainConfig() const noexcept
      -> const engine::vulkan::SwapchainConfig & {
    return swapchainConfig;
  }

  [[nodiscard]] auto getCore() const noexcept
      -> const engine::rendering::Core & {
    return core;
  }

  [[nodiscard]] auto getCore() noexcept -> engine::rendering::Core & {
    return core;
  }

  [[nodiscard]] auto getPhysicalDevice() const noexcept
      -> const vk::raii::PhysicalDevice & {
    return physicalDevice;
  }

  [[nodiscard]] auto getSwapchain() const noexcept
      -> const engine::vulkan::Swapchain & {
    return swapchain;
  }

  [[nodiscard]] auto getDevice() const noexcept -> const vk::raii::Device & {
    return device;
  }

  [[nodiscard]] auto getQueues() const noexcept -> const Queues & {
    return queues;
  }

  [[nodiscard]] auto getSyncObjects(const uint32_t frameIndex) const noexcept
      -> const SyncObjects & {
    return syncObjects[frameIndex];
  }

  [[nodiscard]] auto getCurrentCommandPool() const noexcept
      -> const vk::raii::CommandPool & {
    return commandPool;
  }

  virtual void endFrame() noexcept;

  [[nodiscard]] auto allocCmdBuffer(const vk::CommandBufferLevel level,
                                    const uint32_t count) const noexcept
      -> std::expected<std::vector<vk::raii::CommandBuffer>, std::string> {
    vk::CommandBufferAllocateInfo allocInfo{.commandPool = *commandPool,
                                            .level = level,
                                            .commandBufferCount = count};

    VK_MAKE(cmdBuffers, device.allocateCommandBuffers(allocInfo),
            "Failed to allocate command buffers");

    return std::move(cmdBuffers);
  }

  void checkSwapchain() noexcept {
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

  auto getNextImage() noexcept -> std::expected<
      std::pair<uint32_t,
                std::pair<uint32_t, engine::vulkan::Swapchain::State>>,
      std::string> {
    auto &sync = getSyncObjects(currentFrame);
    auto res = swapchain.getNextImage(device, sync.drawingFence,
                                      sync.presentCompleteSemaphore);
    if (res.has_value()) {

      auto val = std::make_pair(currentFrame, res.value());
      currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
      return std::move(val);
    }

    return std::unexpected(res.error());
  }

protected:
  Input _input;
  engine::rendering::Core core;

  vk::raii::PhysicalDevice physicalDevice;
  vk::raii::Device device;
  Queues queues;

  engine::vulkan::SwapchainConfig swapchainConfig;
  engine::vulkan::Swapchain swapchain;

  vk::raii::CommandPool commandPool;

  std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects;

  int currentFrame = 0;

  struct OldSwapchain {
    engine::vulkan::Swapchain swapchain;
    int frameIndex;
  };

  std::optional<OldSwapchain> oldSwapchain = std::nullopt;

  App(engine::rendering::Core &&core, vk::raii::PhysicalDevice &&physicalDevice,
      vk::raii::Device &&device, Queues &&queues,
      engine::vulkan::SwapchainConfig &&swapchainConfig,
      engine::vulkan::Swapchain &&swapchain,
      vk::raii::CommandPool &&commandPool,
      std::array<SyncObjects, MAX_FRAMES_IN_FLIGHT> &&syncObjects) noexcept
      : core(std::move(core)), physicalDevice(std::move(physicalDevice)),
        device(std::move(device)), queues(std::move(queues)),
        swapchainConfig(swapchainConfig), swapchain(std::move(swapchain)),
        commandPool(std::move(commandPool)),
        syncObjects(std::move(syncObjects)) {}
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
    app.update(deltaTime);
    app.render();
    app.endFrame();

    lastFrame = now;
  }
}
} // namespace engine
