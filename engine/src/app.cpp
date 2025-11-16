#include "engine/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>

#include <engine/core.hpp>
#include <engine/setup.hpp>
#include <engine/util/macros.hpp>
#include <vkh/physicalDeviceSelector.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>

namespace engine {

std::expected<App::FrameInfo, App::TickResult> App::newFrame() noexcept {
  checkSwapchain();
  auto nextImage_res = getNextImage();
  if (!nextImage_res) {
    Logger::critical("Failed to acquire next image: {}", nextImage_res.error());
    return std::unexpected(App::TickResult::Bail);
  }

  auto &[frameIndex, nextImage] = nextImage_res.value();
  auto &[imageIndex, state] = nextImage;

  if (state == vkh::Swapchain::State::OutOfDate ||
      state == vkh::Swapchain::State::Suboptimal) {
    Logger::warn("Swapchain is out of date, recreating it.");
    auto res = recreateSwapchain();
    if (!res) {
      Logger::critical("Failed to recreate swapchain: {}", res.error());
      return std::unexpected(App::TickResult::Bail);
    }
    return std::unexpected(App::TickResult::Recoverable);
  }

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  return App::FrameInfo{.frameIndex = frameIndex, .imageIndex = imageIndex};
}

App::TickResult
App::presentFrame(FrameInfo frameInfo,
                  std::span<vk::CommandBuffer> cmdBuffers) noexcept {
  vk::PipelineStageFlags waitStage(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const auto &so = this->syncObjects[frameInfo.frameIndex];
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*so.presentCompleteSemaphore,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = cmdBuffers.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*so.renderCompleteSemaphore};

  queues.graphics.queue->submit(submitInfo, *so.drawingFence);

  const vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                       .pWaitSemaphores =
                                           &*so.renderCompleteSemaphore,
                                       .swapchainCount = 1,
                                       .pSwapchains = &**swapchain,
                                       .pImageIndices = &frameInfo.imageIndex};

  auto result = static_cast<vk::Result>(
      queues.present.queue->getDispatcher()->vkQueuePresentKHR(
          **queues.present.queue, &*presentInfo));

  if (result == vk::Result::eSuboptimalKHR ||
      result == vk::Result::eErrorOutOfDateKHR) {
    Logger::warn("Swapchain is suboptimal, recreating it.");
    auto res = recreateSwapchain();
    if (!res) {
      Logger::critical("Failed to recreate swapchain: {}", res.error());
      return TickResult::Bail;
    }

    return TickResult::Recoverable;
  }

  if (result != vk::Result::eSuccess) {
    Logger::error("Failed to present image: {}", vk::to_string(result));
    return TickResult::Recoverable;
  }

  return TickResult::Success;
}

void App::endFrame() noexcept {
  Input::instance().onFrameEnd();
  ImGui::EndFrame();
}

void App::drawImGui(vk::raii::CommandBuffer &cmdBuffer,
                    uint32_t imageIndex) const noexcept {
  ImGui::Render();

  vk::RenderingAttachmentInfo attachmentInfo{
      .imageView = swapchain.nImageView(imageIndex),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eLoad,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = {std::array<float, 4>{
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       1.0f,
                                   }}}};

  vk::RenderingInfo renderingInfo{
      .renderArea =
          vk::Rect2D{.offset = {.x = 0, .y = 0},
                     .extent = {.width = swapchain.config().extent.width,
                                .height = swapchain.config().extent.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};

  cmdBuffer.beginRendering(renderingInfo);

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmdBuffer);

  cmdBuffer.endRendering();
}

auto App::recreateSwapchain() noexcept -> std::expected<void, std::string> {
  Logger::trace("Recreating Swapchain");

  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  EG_MAKE(newSwapchain,
          setup::createSwapchain(physicalDevice, device, window, surface,
                                 {queues.graphics.index, queues.present.index},
                                 &*swapchain),
          "Failed to create new swapchain");
  oldSwapchain = OldSwapchain{
      .swapchain = std::move(swapchain),
      .frameIndex = currentFrame,
  };

  swapchain = std::move(newSwapchain);

  Logger::trace("Swapchain recreated successfully");
  return {};
}

void App::setupCmdBuffer(
    const vk::raii::CommandBuffer &cmdBuffer) const noexcept {
  cmdBuffer.setViewport(
      0, vk::Viewport{.x = 0.0f,
                      .y = 0.0f,
                      .width = static_cast<float>(renderImage.extent.width),
                      .height = static_cast<float>(renderImage.extent.height),
                      .minDepth = 1.0f,
                      .maxDepth = 0.0f});

  cmdBuffer.setScissor(
      0, vk::Rect2D{.offset = {.x = 0, .y = 0},
                    .extent = {.width = renderImage.extent.width,
                               .height = renderImage.extent.height}});
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

    SwapchainImageResult val = {.thisFrame = currentFrame, .img = res.value()};
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return val;
  }

  return std::unexpected(res.error());
}

} // namespace engine
