#include "app/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>

#include "logger.hpp"
#include "vulkan/vulkan.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/util/macros.hpp>
#include <vkh/physicalDeviceSelector.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>

#include <imgui/imgui.h>

void App::onWindowResize(engine::Dimensions dim) noexcept {
  Logger::info("Window resized to {}x{}", dim.width, dim.height);
  camera.camera.onResize(dim.width, dim.height);
}

App::TickResult App::update(float deltaTime) noexcept {
  engine::FrameData frameData{
      .deltaTimeMs = deltaTime,
      .input = engine::Input::instance(),
  };

  camera.camera.update(frameData);

  return TickResult::Success;
}

App::TickResult App::render() noexcept {
  auto res = newFrame();
  if (!res) {
    return res.error();
  }
  auto fInfo = res.value();

  auto &cmdBuffer = commandBuffers[fInfo.frameIndex];

  camera.camera.writeMatrices(camera.buffers, fInfo.frameIndex);

  cmdBuffer.begin(vk::CommandBufferBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  engine::transitionImageLayout(
      cmdBuffer, renderImage.image, vk::ImageLayout::eUndefined,
      vk::ImageLayout::eColorAttachmentOptimal, {},
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eTopOfPipe,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::RenderingAttachmentInfo attachmentInfo{
      .imageView = renderImage.view,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = vk::ClearValue{.color = {std::array<float, 4>{
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       1.0f,
                                   }}}};

  vk::RenderingInfo renderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                               .extent = {.width = renderImage.extent.width,
                                          .height = renderImage.extent.height}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &attachmentInfo};

  Logger::trace("Beginning rendering");
  cmdBuffer.beginRendering(renderingInfo);

  draw(cmdBuffer, fInfo.frameIndex);

  cmdBuffer.endRendering();

  engine::transitionImageLayout(
      cmdBuffer, renderImage.image, vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::eTransferSrcOptimal,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::AccessFlagBits2::eTransferRead,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eTransfer);

  engine::transitionImageLayout(cmdBuffer, swapchain.images()[fInfo.imageIndex],
                                vk::ImageLayout::eUndefined,
                                vk::ImageLayout::eTransferDstOptimal, {},
                                vk::AccessFlagBits2::eTransferWrite,
                                vk::PipelineStageFlagBits2::eTopOfPipe,
                                vk::PipelineStageFlagBits2::eTransfer);

  vk::ImageBlit2 blit{
      .srcSubresource =
          vk::ImageSubresourceLayers{
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .srcOffsets = {{
          vk::Offset3D{.x = 0, .y = 0, .z = 0},
          vk::Offset3D{
              .x = static_cast<int32_t>(renderImage.extent.width),
              .y = static_cast<int32_t>(renderImage.extent.height),
              .z = 1,
          },
      }},
      .dstSubresource =
          vk::ImageSubresourceLayers{
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
      .dstOffsets = {{
          vk::Offset3D{.x = 0, .y = 0, .z = 0},
          vk::Offset3D{
              .x = static_cast<int32_t>(swapchain.config().extent.width),
              .y = static_cast<int32_t>(swapchain.config().extent.height),
              .z = 1,
          },
      }}};

  vk::BlitImageInfo2 blitInfo{
      .srcImage = renderImage.image,
      .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
      .dstImage = swapchain.images()[fInfo.imageIndex],
      .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
      .regionCount = 1,
      .pRegions = &blit,
      .filter = vk::Filter::eLinear,
  };

  cmdBuffer.blitImage2(blitInfo);

  {
    ImGui::ShowDemoWindow();
  }

  ui();

  engine::transitionImageLayout(
      cmdBuffer, swapchain.images()[fInfo.imageIndex],
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::AccessFlagBits2::eTransferWrite,
      vk::AccessFlagBits2::eColorAttachmentWrite,
      vk::PipelineStageFlagBits2::eTransfer,
      vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  drawImGui(cmdBuffer, fInfo.imageIndex);

  engine::transitionImageLayout(
      cmdBuffer, swapchain.images()[fInfo.imageIndex],
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::AccessFlagBits2::eColorAttachmentWrite, {},
      vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      vk::PipelineStageFlagBits2::eBottomOfPipe);

  cmdBuffer.end();

  auto cmdBuf = static_cast<vk::CommandBuffer>(cmdBuffer);

  return presentFrame(fInfo, {&cmdBuf, 1});
}

void App::draw(vk::raii::CommandBuffer &cmdBuffer, uint32_t frameIndex) {
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

  setupCmdBuffer(cmdBuffer);

  cmdBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, pipeline.getLayout(), 0,
      {camera.buffers.descriptorSets[frameIndex]}, nullptr);

  pipelines::Mesh::MeshPushConstants pc{
      .modelMatrix = glm::mat4(1.0f),
      .vBufferAddress = vertexBufferAddress,
  };

  cmdBuffer.pushConstants<pipelines::Mesh::MeshPushConstants>(
      *pipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0, pc);

  cmdBuffer.draw(4, 1, 0, 0);
}

void App::ui() {
  ImGui::Begin("Info");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Text("Camera position: (%.2f, %.2f, %.2f)",
              camera.camera.getPosition().x, camera.camera.getPosition().y,
              camera.camera.getPosition().z);
  ImGui::Text("Camera rotation: (Yaw: %.2f, Pitch: %.2f)",
              camera.camera.getRotation().yaw,
              camera.camera.getRotation().pitch);
  ImGui::End();
}
