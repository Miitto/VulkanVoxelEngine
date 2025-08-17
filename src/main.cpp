#include "app/app.hpp"
#include <optional>

#include "logger.hpp"
#include "pipelines/pipelines.hpp"

#include <engine/util/window_manager.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <vulkan/vulkan_raii.hpp>

class Program {
  App app;
  pipelines::GreedyVoxel pipeline;

  std::vector<vk::raii::CommandBuffer> commandBuffers;

  Program(App &app, pipelines::GreedyVoxel &pipeline,
          std::vector<vk::raii::CommandBuffer> &commandBuffers)
      : app(std::move(app)), pipeline(std::move(pipeline)),
        commandBuffers(std::move(commandBuffers)) {}

public:
  static auto create() -> std::expected<Program, std::string> {
    auto app_opt = App::create();
    if (!app_opt.has_value()) {
      Logger::critical("Failed to create App.");
      return std::unexpected(app_opt.error());
    }

    auto &app = app_opt.value();

    auto cmdBuffers_res = app.allocCmdBuffer(vk::CommandBufferLevel::ePrimary,
                                             MAX_FRAMES_IN_FLIGHT);
    if (!cmdBuffers_res) {
      Logger::critical("Failed to allocate command buffer: {}",
                       cmdBuffers_res.error());
      return std::unexpected(cmdBuffers_res.error());
    }

    auto &cmdBuffers = cmdBuffers_res.value();

    auto shader_res =
        engine::vulkan::Shader::create(app.getDevice(), "basic.spv");

    if (!shader_res) {
      Logger::error("Failed to create shader module: {}", shader_res.error());
      return std::unexpected(shader_res.error());
    }

    auto &shaderModule = shader_res.value();

    [[maybe_unused]]
    auto pipelineShaderStages = shaderModule.vertFrag();

    auto greedyVoxel_res = pipelines::GreedyVoxel::create(
        app.getDevice(), app.getSwapchainConfig());

    auto &pipeline = greedyVoxel_res.value();

    return Program(app, pipeline, cmdBuffers);
  }

  void closing() {
    Logger::info("Closing application...");

    app.getDevice().waitIdle();

    Logger::info("Application closed successfully.");
  }

  void run() {
    Logger::info("Starting application...");
    app.getCore().getWindow().setResizeCallback(
        this, [](engine::Dimensions dim, void *data) {
          Logger::trace("Window resized to {}x{}", dim.width, dim.height);
          auto *self = reinterpret_cast<Program *>(data);

          self->redraw();
        });

    while (!app.shouldClose()) {
      app.poll();

      if (redraw()) {
        Logger::error("Redraw failed, exiting...");
        break;
      }

      app.endFrame();
    }
  }

  auto redraw() -> bool {
    if (!update()) {
      Logger::error("Update failed, exiting...");
      return true;
    }

    if (!render()) {
      Logger::error("Render failed, exiting...");
      return true;
    }

    return false;
  }

  auto update() -> bool { return true; }

  auto render() -> bool {
    Logger::trace("Rendering frame...");
    app.checkSwapchain();
    auto nextImage_res = app.getNextImage();
    if (!nextImage_res) {
      Logger::error("Failed to acquire next image: {}", nextImage_res.error());
      return false;
    }
    Logger::trace("Acquired next image successfully.");

    auto &[imageIndex, state] = nextImage_res.value();
    if (state == engine::vulkan::Swapchain::State::OutOfDate) {
      Logger::warn("Swapchain is out of date, recreating it.");
      auto res = app.recreateSwapchain();
      if (!res) {
        Logger::error("Failed to recreate swapchain: {}", res.error());
        return false;
      }
      return true;
    }
    Logger::trace("Image index: {}", imageIndex);

    auto &cmdBuffer = commandBuffers[app.frameIndex()];

    cmdBuffer.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    app.preRender(cmdBuffer, imageIndex);

    vk::RenderingAttachmentInfo attachmentInfo{
        .imageView = app.getSwapchain()[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)};

    vk::RenderingInfo renderingInfo{
        .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0},
                                 .extent = app.getSwapchainConfig().extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo};

    Logger::trace("Beginning rendering");
    cmdBuffer.beginRendering(renderingInfo);

    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    cmdBuffer.setViewport(
        0,
        vk::Viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(app.getSwapchainConfig().extent.width),
            .height =
                static_cast<float>(app.getSwapchainConfig().extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f});

    cmdBuffer.setScissor(0,
                         vk::Rect2D{.offset = {.x = 0, .y = 0},
                                    .extent = app.getSwapchainConfig().extent});

    cmdBuffer.draw(3, 1, 0, 0);

    cmdBuffer.endRendering();

    app.postRender(cmdBuffer, imageIndex);

    cmdBuffer.end();

    vk::PipelineStageFlags waitStage(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const auto &syncObjects = app.getSyncObjects();
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*syncObjects.presentCompleteSemaphore,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &*cmdBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*syncObjects.renderCompleteSemaphore};

    const auto &queues = app.getQueues();

    Logger::trace("Submitting command buffer to graphics queue");
    queues.graphicsQueue.queue->submit(submitInfo, *syncObjects.drawingFence);

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*syncObjects.renderCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &**app.getSwapchain(),
        .pImageIndices = &imageIndex};

    Logger::trace("Presenting image to swapchain");
    auto result = static_cast<vk::Result>(
        queues.presentQueue.queue->getDispatcher()->vkQueuePresentKHR(
            **queues.presentQueue.queue, &*presentInfo));

    if (state == engine::vulkan::Swapchain::State::Suboptimal ||
        result == vk::Result::eSuboptimalKHR ||
        result == vk::Result::eErrorOutOfDateKHR) {
      Logger::warn("Swapchain is suboptimal, consider recreating it.");
      auto res = app.recreateSwapchain();
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
};

auto main() -> int {
  Logger::init();
  auto windowManager = WindowManager();

  auto app = Program::create();

  if (!app.has_value()) {
    Logger::critical("Failed to create Program.");
    return EXIT_FAILURE;
  }

  app->run();

  app->closing();

  glfwTerminate();

  Logger::info("Application terminated successfully.");
  return EXIT_SUCCESS;
}
