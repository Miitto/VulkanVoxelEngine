#include "app/app.hpp"

#include "engine/vulkan/memorySelector.hpp"
#include "logger.hpp"
#include "pipelines/pipelines.hpp"
#include "vertex.hpp"
#include <engine/util/macros.hpp>

#include "camera.hpp"
#include <engine/util/window_manager.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

class Program {
  App app;
  pipelines::GreedyVoxel pipeline;

  std::vector<vk::raii::CommandBuffer> commandBuffers;

  vk::raii::Buffer vertexBuffer;
  vk::raii::DeviceMemory vBufferMemory;

  vk::raii::DescriptorPool cameraDescriptorPool;
  PerspectiveCamera::Buffers cameraBuffers;

  PerspectiveCamera camera;

  Program(App &app, pipelines::BasicVertex &pipeline,
          std::vector<vk::raii::CommandBuffer> &commandBuffers,
          vk::raii::Buffer &vertexBuffer, vk::raii::DeviceMemory &vBufferMemory,
          vk::raii::DescriptorPool &cameraDescriptorPool,
          PerspectiveCamera::Buffers &cameraBuffers, PerspectiveCamera &camera)
      : app(std::move(app)), pipeline(std::move(pipeline)),
        commandBuffers(std::move(commandBuffers)),
        vertexBuffer(std::move(vertexBuffer)),
        vBufferMemory(std::move(vBufferMemory)),
        cameraDescriptorPool(std::move(cameraDescriptorPool)),
        cameraBuffers(std::move(cameraBuffers)), camera(std::move(camera)) {}

public:
  static auto create() noexcept -> std::expected<Program, std::string> {
    auto app_opt = App::create();
    if (!app_opt.has_value()) {
      Logger::critical("Failed to create App.");
      return std::unexpected(app_opt.error());
    }

    auto &app = app_opt.value();

    EG_MAKE(commandBuffers,
            app.allocCmdBuffer(vk::CommandBufferLevel::ePrimary,
                               MAX_FRAMES_IN_FLIGHT),
            "Failed to allocate command buffers");

    std::array<Vertex, 4> vertices = {
        Vertex{.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
        Vertex{.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
        Vertex{.position = {-0.5f, 0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
        Vertex{.position = {0.5f, 0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}}};

    vk::BufferCreateInfo vertexBufferInfo{
        .size = sizeof(vertices[0]) * vertices.size(),
        .usage = vk::BufferUsageFlagBits::eVertexBuffer,
        .sharingMode = vk::SharingMode::eExclusive};

    VK_MAKE(vertexBuffer, app.getDevice().createBuffer(vertexBufferInfo),
            "Failed to create vertex buffer");

    auto memSelector =
        engine::vulkan::MemorySelector(vertexBuffer, app.getPhysicalDevice());

    EG_MAKE(vertexAllocInfo,
            memSelector.allocInfo(vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent),
            "Failed to get memory allocation info");

    VK_MAKE(vBufferMemory, app.getDevice().allocateMemory(vertexAllocInfo),
            "Failed to allocate vertex buffer memory");

    vertexBuffer.bindMemory(*vBufferMemory, 0);

    void *data = vBufferMemory.mapMemory(0, vertexBufferInfo.size);
    memcpy(data, vertices.data(), vertexBufferInfo.size);
    vBufferMemory.unmapMemory();

    vk::DescriptorPoolSize poolSize{.type = vk::DescriptorType::eUniformBuffer,
                                    .descriptorCount = MAX_FRAMES_IN_FLIGHT};
    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize};
    VK_MAKE(cameraDescriptorPool,
            app.getDevice().createDescriptorPool(poolInfo),
            "Failed to create camera descriptor pool");

    EG_MAKE(cameraDescriptorLayout,
            PerspectiveCamera::descriptorLayout(app.getDevice()),
            "Failed to create camera descriptor layout");

    EG_MAKE(cameraBuffers,
            PerspectiveCamera::createBuffers(
                app.getDevice(), app.getPhysicalDevice(), cameraDescriptorPool,
                cameraDescriptorLayout),
            "Failed to create uniform buffers");

    EG_MAKE(basicVertexPipeline,
            pipelines::BasicVertex::create(
                app.getDevice(), app.getSwapchainConfig(),
                pipelines::BasicVertex::DescriptorLayouts{
                    .camera = cameraDescriptorLayout}),
            "Failed to create basic vertex pipeline");

    PerspectiveCamera camera(
        {0.0f, 0.0f, 2.0f}, {},
        engine::cameras::Perspective::Params{
            .fov = glm::radians(90.0f),
            .aspectRatio =
                static_cast<float>(app.getSwapchainConfig().extent.width) /
                static_cast<float>(app.getSwapchainConfig().extent.height),
            .nearPlane = 0.1f,
            .farPlane = 100.0f});

    return Program(app, basicVertexPipeline, commandBuffers, vertexBuffer,
                   vBufferMemory, cameraDescriptorPool, cameraBuffers, camera);
  }

  void closing() {
    Logger::info("Closing application...");

    app.getDevice().waitIdle();

    Logger::info("Application closed successfully.");
  }

  void run() {
    Logger::info("Starting application...");
    auto &window = app.getCore().getWindow();
    window.setResizeCallback(this, [](engine::Dimensions dim, void *data) {
      Logger::trace("Window resized to {}x{}", dim.width, dim.height);
      auto *self = reinterpret_cast<Program *>(data);

      self->camera.onResize(dim.width, dim.height);
      self->redraw();
    });

    window.setKeyCallback(
        this, [](int key, int scancode, int action, int mods, void *data) {
          (void)scancode;
          (void)mods;
          auto *self = reinterpret_cast<Program *>(data);
          if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            self->app.getCore().getWindow().close();
          }
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

    auto &[frameIndex, nextImage] = nextImage_res.value();
    auto &[imageIndex, state] = nextImage;

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

    auto &cmdBuffer = commandBuffers[frameIndex];

    [[maybe_unused]]
    auto offset = frameIndex * sizeof(engine::Camera::Matrices);

    auto cameraMatrices = camera.matrices();
    memcpy((char *)cameraBuffers.mapping + offset, &cameraMatrices,
           sizeof(engine::Camera::Matrices));

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

    app.setupCmdBuffer(cmdBuffer);

    cmdBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                 pipeline.getLayout(), 0,
                                 {cameraBuffers.descriptorSets[0]}, nullptr);

    cmdBuffer.draw(4, 1, 0, 0);

    cmdBuffer.endRendering();

    app.postRender(cmdBuffer, imageIndex);

    cmdBuffer.end();

    vk::PipelineStageFlags waitStage(
        vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const auto &syncObjects = app.getSyncObjects(frameIndex);
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

auto main() noexcept -> int {
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
