#include "app/app.hpp"
#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <optional>

#include <vk/structs/rect2d.hpp>
#include <vk/structs/viewport.hpp>

#include "logger.hpp"

class Program {
  App app;

  Program(App &app) : app(std::move(app)) {}

public:
  static auto create() -> std::optional<Program> {
    auto app_opt = App::create();
    if (!app_opt.has_value()) {
      Logger::critical("Failed to create App.");
      return std::nullopt;
    }

    return Program(app_opt.value());
  }

  void run() {
    Logger::info("Starting application...");
    while (!app.window.shouldClose()) {
      app.poll();

      if (!update())
        return;
      if (!render())
        return;
    }
  }

  auto update() -> bool { return true; }

  auto render() -> bool {
    app.frames[app.currentFrame].inFlight.wait();
    auto [swpachainState, imageIndex] = app.swapchain.getNextImage(
        &app.frames[app.currentFrame].imageAvailable);

    switch (swpachainState) {
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
      Logger::trace("Recreate swapchain");
      return false;
    case VK_SUCCESS:
      break;
    default: {
      Logger::critical("Failed to acquire swapchain image.");
      return false;
    }
    }
    app.frames[app.currentFrame].inFlight.reset();

    auto &uniform = app.uniforms.objects[app.currentFrame];

    CameraMatrices matrices{
        .model = glm::mat4(1.0f),
        .view = glm::lookAt(glm::vec3(3., 3., 3.), {0., 0., 0.}, {0., 0., 1.}),
        .projection = glm::perspective(
            glm::radians(90.0f),
            static_cast<float>(app.swapchain.getExtent().width) /
                static_cast<float>(app.swapchain.getExtent().height),
            0.1f, 100.0f),
    };
    matrices.projection[1][1] *= -1; // Vulkan uses a different Y-axis

    if (!uniform.bufferMapping.write(&matrices, vk::Size(sizeof(matrices))))
      return false;

    app.frames[app.currentFrame].commandBuffer.reset();
    recordCommandBuffer(app.frames[app.currentFrame].commandBuffer, imageIndex);

    vk::info::Submit submitInfo;
    submitInfo
        .addWaitSemaphore(*app.frames[app.currentFrame].imageAvailable,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
        .addSignalSemaphore(*app.frames[app.currentFrame].renderFinished)
        .addCommandBuffer(*app.frames[app.currentFrame].commandBuffer);

    if (app.graphicsQueue
            .submit(submitInfo, &app.frames[app.currentFrame].inFlight)
            .has_value()) {
      Logger::critical("Failed to submit draw command buffer.");
      return false;
    }

    vk::info::Present presentInfo =
        vk::info::Present()
            .addWaitSemaphore(*app.frames[app.currentFrame].renderFinished)
            .addSwapchain(*app.swapchain)
            .setImageIndex(imageIndex);

    if (app.presentQueue.present(presentInfo) != VK_SUCCESS) {
      Logger::critical("Failed to present swapchain image");
      return false;
    }

    app.currentFrame = (app.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
  }

  void recordCommandBuffer(vk::CommandBuffer &commandBuffer,
                           uint32_t imageIndex) {
    auto encoder = commandBuffer.begin();

    vk::info::RenderPassBegin renderPassInfo(
        *app.renderPass, *app.framebuffers[imageIndex],
        {.offset = {0, 0}, .extent = app.swapchain.getExtent()});

    renderPassInfo.addClearValue({.color = {{0.0f, 0.0f, 0.0f, 1.0f}}});

    auto pass = encoder.beginRenderPass(renderPassInfo);

    pass.bindPipeline(app.pipeline);

    vk::Viewport viewport(static_cast<float>(app.swapchain.getExtent().width),
                          static_cast<float>(app.swapchain.getExtent().height));

    vk::Rect2D scissor(app.swapchain.getExtent());

    pass.setViewport(viewport);
    pass.setScissor(scissor);

    pass.bindVertexBuffer(0, app.vertexBuffer.vertexBuffer);
    pass.bindIndexBuffer(app.vertexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    pass.bindDescriptorSet(
        app.uniforms.objects[app.currentFrame].descriptorSet);

    pass.drawIndexed(6);

    pass.end();

    encoder.end();
  }
};

auto main() -> int {
  engine::init();
  Logger::init();
  auto app = Program::create();

  if (!app.has_value()) {
    Logger::critical("Failed to create Program.");
    return EXIT_FAILURE;
  }

  app->run();

  engine::terminate();

  Logger::info("Application terminated successfully.");
  return EXIT_SUCCESS;
}
