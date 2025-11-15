#include "engine/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "vkh/queueFinder.hpp"

#include <engine/core.hpp>
#include <engine/setup.hpp>
#include <engine/util/macros.hpp>
#include <vkh/physicalDeviceSelector.hpp>

namespace engine {
void App::endFrame() noexcept { _input.onFrameEnd(); }

auto App::recreateSwapchain() noexcept -> std::expected<void, std::string> {
  Logger::trace("Recreating Swapchain");

  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  EG_MAKE(newSwapchainParts,
          setup::createSwapchain(physicalDevice, device, window, surface,
                                 {queues.graphics.index, queues.present.index},
                                 &*swapchain),
          "Failed to create new swapchain");
  auto &[newSwapchainConfig, newSwapchain] = newSwapchainParts;

  oldSwapchain = OldSwapchain{
      .swapchain = std::move(swapchain),
      .frameIndex = currentFrame,
  };

  swapchainConfig = newSwapchainConfig;
  swapchain = std::move(newSwapchain);

  Logger::trace("Swapchain recreated successfully");
  return {};
}

void App::preRender(const vk::raii::CommandBuffer &commandBuffer,
                    const size_t index) noexcept {
  engine::transitionImageLayout(
      commandBuffer, swapchain.nImage(index), vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal, {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eTopOfPipe,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);
}

void App::postRender(const vk::raii::CommandBuffer &commandBuffer,
                     const size_t index) noexcept {
  engine::transitionImageLayout(
      commandBuffer, swapchain.nImage(index),
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite, {},
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe);
}

void App::setupCmdBuffer(
    const vk::raii::CommandBuffer &cmdBuffer) const noexcept {
  cmdBuffer.setViewport(
      0,
      vk::Viewport{.x = 0.0f,
                   .y = 0.0f,
                   .width = static_cast<float>(swapchainConfig.extent.width),
                   .height = static_cast<float>(swapchainConfig.extent.height),
                   .minDepth = 1.0f,
                   .maxDepth = 0.0f});

  cmdBuffer.setScissor(0, vk::Rect2D{.offset = {.x = 0, .y = 0},
                                     .extent = swapchainConfig.extent});
}

auto App::allocCmdBuffer(const vk::CommandBufferLevel level,
                         const uint32_t count) const noexcept
    -> std::expected<std::vector<vk::raii::CommandBuffer>, std::string> {
  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = *commandPool, .level = level, .commandBufferCount = count};

  VK_MAKE(cmdBuffers, device.allocateCommandBuffers(allocInfo),
          "Failed to allocate command buffers");

  return std::move(cmdBuffers);
}

void App::checkSwapchain() noexcept {
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

auto App::getNextImage() noexcept
    -> std::expected<SwapchainImageResult, std::string> {
  auto &sync = syncObjects[currentFrame];
  auto res = swapchain.getNextImage(device, sync.drawingFence,
                                    sync.presentCompleteSemaphore);
  if (res.has_value()) {

    SwapchainImageResult val = {currentFrame, res.value()};
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return std::move(val);
  }

  return std::unexpected(res.error());
}

} // namespace engine
