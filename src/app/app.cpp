#include "app/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "logger.hpp"
#include "vkh/queueFinder.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/util/macros.hpp>
#include <vertex.hpp>
#include <vkh/memorySelector.hpp>
#include <vkh/physicalDeviceSelector.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>

void App::onWindowResize(engine::Dimensions dim) noexcept {
  Logger::info("Window resized to {}x{}", dim.width, dim.height);
  camera.camera.onResize(dim.width, dim.height);
}

bool App::update(float deltaTime) noexcept {
  engine::FrameData frameData{
      .deltaTimeMs = deltaTime,
      .input = _input,
  };

  camera.camera.update(frameData);

  return true;
}

bool App::render() noexcept {
  Logger::trace("Rendering frame...");
  checkSwapchain();
  auto nextImage_res = getNextImage();
  if (!nextImage_res) {
    Logger::error("Failed to acquire next image: {}", nextImage_res.error());
    return false;
  }
  Logger::trace("Acquired next image successfully.");

  auto &[frameIndex, nextImage] = nextImage_res.value();
  auto &[imageIndex, state] = nextImage;

  if (state == vkh::Swapchain::State::OutOfDate) {
    Logger::warn("Swapchain is out of date, recreating it.");
    auto res = recreateSwapchain();
    if (!res) {
      Logger::error("Failed to recreate swapchain: {}", res.error());
      return false;
    }
    return true;
  }
  Logger::trace("Image index: {}", imageIndex);

  auto &cmdBuffer = commandBuffers[frameIndex];

  [[maybe_unused]]
  auto offset = frameIndex * sizeof(engine::Camera::Matrices);

  auto cameraMatrices = camera.camera.matrices();
  memcpy((char *)camera.buffers.mapping + offset, &cameraMatrices,
         sizeof(engine::Camera::Matrices));

  cmdBuffer.begin(vk::CommandBufferBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  preRender(cmdBuffer, imageIndex);

  vk::RenderingAttachmentInfo attachmentInfo{
      .imageView = swapchain[imageIndex],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)};

  vk::RenderingInfo renderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                               .extent = swapchainConfig.extent},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};

  Logger::trace("Beginning rendering");
  cmdBuffer.beginRendering(renderingInfo);

  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

  setupCmdBuffer(cmdBuffer);

  cmdBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               pipeline.getLayout(), 0,
                               {camera.buffers.descriptorSets[0]}, nullptr);

  cmdBuffer.draw(4, 1, 0, 0);

  cmdBuffer.endRendering();

  postRender(cmdBuffer, imageIndex);

  cmdBuffer.end();

  vk::PipelineStageFlags waitStage(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);

  const auto &so = this->syncObjects[frameIndex];
  const vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*so.presentCompleteSemaphore,
      .pWaitDstStageMask = &waitStage,
      .commandBufferCount = 1,
      .pCommandBuffers = &*cmdBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*so.renderCompleteSemaphore};

  Logger::trace("Submitting command buffer to graphics queue");
  queues.graphics.queue->submit(submitInfo, *so.drawingFence);

  const vk::PresentInfoKHR presentInfo{.waitSemaphoreCount = 1,
                                       .pWaitSemaphores =
                                           &*so.renderCompleteSemaphore,
                                       .swapchainCount = 1,
                                       .pSwapchains = &**swapchain,
                                       .pImageIndices = &imageIndex};

  Logger::trace("Presenting image to swapchain");
  auto result = static_cast<vk::Result>(
      queues.present.queue->getDispatcher()->vkQueuePresentKHR(
          **queues.present.queue, &*presentInfo));

  if (state == vkh::Swapchain::State::Suboptimal ||
      result == vk::Result::eSuboptimalKHR ||
      result == vk::Result::eErrorOutOfDateKHR) {
    Logger::warn("Swapchain is suboptimal, consider recreating it.");
    auto res = recreateSwapchain();
    if (!res) {
      Logger::error("Failed to recreate swapchain: {}", res.error());
      return false;
    }

    return true;
  }

  if (result != vk::Result::eSuccess) {
    Logger::error("Failed to present image: {}", vk::to_string(result));
    return false;
  }

  Logger::trace("Frame rendered successfully.");
  return true;
}
