#include "app/app.hpp"

#include "camera.hpp"
#include "engine/vulkan/memorySelector.hpp"
#include "glm/geometric.hpp"
#include "input.hpp"
#include "logger.hpp"
#include "pipelines/pipelines.hpp"
#include "vertex.hpp"
#include <engine/util/macros.hpp>
#include <engine/util/window_manager.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

class Program {
  App app;
  Input input;

  pipelines::GreedyVoxel pipeline;

  std::vector<vk::raii::CommandBuffer> commandBuffers;

  vk::raii::Buffer vertexBuffer;
  vk::raii::DeviceMemory vBufferMemory;

  vk::raii::DescriptorPool cameraDescriptorPool;
  PerspectiveCamera::Buffers cameraBuffers;

  PerspectiveCamera camera;

  std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;

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
        {0.0f, 0.0f, 2.0f}, {1.0f, 0.0f, 0.0f, 0.0f},
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
    window.setResizeCallback([this](engine::Dimensions dim) {
      Logger::trace("Window resized to {}x{}", dim.width, dim.height);
      camera.onResize(dim.width, dim.height);

      auto now = std::chrono::high_resolution_clock::now();

      auto deltaTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              now - lastFrameTime)
              .count();

      redraw(deltaTime);

      lastFrameTime = now;
    });

    window.setKeyCallback([this, &window](engine::Key key, int scancode,
                                          engine::KeyAction action, int mods) {
      (void)scancode;
      (void)mods;

      using engine::Key;

      switch (key) {
      case Key::Esc:
        if (window.getCursorMode() == engine::CursorMode::Disabled) {
          window.setCursorMode(engine::CursorMode::Normal);
        } else {
          window.setCursorMode(engine::CursorMode::Disabled);
        }
        break;
      default:
        input.key(key, action);
        break;
      }
    });

    window.setCursorMode(engine::CursorMode::Disabled);

    window.setMouseMoveCallback([this](double xpos, double ypos) {
      input.mouseMut().move(
          glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
    });

    lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!app.shouldClose()) {
      app.poll();

      auto now = std::chrono::high_resolution_clock::now();

      auto deltaTime =
          std::chrono::duration<float, std::chrono::seconds::period>(
              now - lastFrameTime)
              .count();

      if (redraw(deltaTime)) {
        Logger::error("Redraw failed, exiting...");
        break;
      }

      app.endFrame();
      input.onFrameEnd();

      lastFrameTime = now;
    }
  }

  auto redraw(float deltaMs) -> bool {
    if (!update(deltaMs)) {
      Logger::error("Update failed, exiting...");
      return true;
    }

    if (!render(deltaMs)) {
      Logger::error("Render failed, exiting...");
      return true;
    }

    return false;
  }

  auto update(float deltaMs) -> bool {
    if (input.isDown(engine::Key::W)) {
      camera.move(glm::vec3(0.0f, 0.0f, -1.f) * deltaMs);
    }
    if (input.isDown(engine::Key::S)) {
      camera.move(glm::vec3(0.0f, 0.0f, 1.f) * deltaMs);
    }
    if (input.isDown(engine::Key::A)) {
      camera.move(glm::vec3(-1.f, 0.0f, 0.0f) * deltaMs);
    }
    if (input.isDown(engine::Key::D)) {
      camera.move(glm::vec3(1.f, 0.0f, 0.0f) * deltaMs);
    }
    if (input.isDown(engine::Key::Space)) {
      camera.moveAbsolute(glm::vec3(0.0f, 1.0f, 0.0f) * deltaMs);
    }
    if (input.isDown(engine::Key::Ctrl)) {
      camera.moveAbsolute(glm::vec3(0.0f, -1.0f, 0.0f) * deltaMs);
    }

    if (input.isPressed(engine::Key::C)) {
      camera.center();
    }

    auto delta = input.mouse().delta();

    if (delta == glm::vec2(0.0f)) {
      return true;
    }

    auto rotationSpeed = 0.0025f;
    auto yaw =
        glm::angleAxis(-delta.x * rotationSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
    auto pitch =
        glm::angleAxis(-delta.y * rotationSpeed, glm::vec3(1.0f, 0.0f, 0.0f));

    auto rot = glm::normalize(pitch * yaw);

    auto camRot = camera.getRotation();

    Logger::debug("Rotation: {}, {}, {}, {}", camRot.w, camRot.x, camRot.y,
                  camRot.z);
    Logger::debug("Rotating by {}, {}, {}, {}", rot.w, rot.x, rot.y, rot.z);
    camera.rotate(rot);

    return true;
  }

  auto render(float deltaMs) -> bool {
    (void)deltaMs;
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
