#include "app.h"
#include "structs/info/commands/renderPassBegin.h"
#include "structs/info/present.h"
#include "structs/info/submit.h"
#include <iostream>

int main() {
  std::cout << "Vulkan Application Starting..." << std::endl;
  auto app = App::create();
  if (!app.has_value()) {
    std::cout << "Failed to create application." << std::endl;
    return EXIT_FAILURE;
  }

  try {
    app->run();
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown exception occurred." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Application terminated successfully." << std::endl;
  return EXIT_SUCCESS;
}

void App::run() {
  std::println("Running app...");
  while (!window.shouldClose()) {
    glfwPollEvents();

    update();
    render();
  }
  std::println("App closed.");
}

void App::update() {}

void App::render() {
  frames[currentFrame].inFlight.wait();
  auto [swpachainState, imageIndex] =
      swapchain.getNextImage(*frames[currentFrame].imageAvailable);

  switch (swpachainState) {
  case VK_SUBOPTIMAL_KHR:
  case VK_ERROR_OUT_OF_DATE_KHR:
    std::println("TODO: Recreate swapchain");
    return;
  case VK_SUCCESS:
    break;
  default: {
    std::cerr << "Failed to acquire swapchain image." << std::endl;
    return;
  }
  }

  frames[currentFrame].inFlight.reset();

  frames[currentFrame].commandBuffer.reset();
  recordCommandBuffer(frames[currentFrame].commandBuffer, imageIndex);

  vk::info::Submit submitInfo;
  submitInfo
      .addWaitSemaphore(*frames[currentFrame].imageAvailable,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
      .addSignalSemaphore(*frames[currentFrame].renderFinished)
      .addCommandBuffer(*frames[currentFrame].commandBuffer);

  if (graphicsQueue.submit(submitInfo, *frames[currentFrame].inFlight) !=
      VK_SUCCESS) {
    std::cerr << "Failed to submit draw command buffer." << std::endl;
    return;
  }

  vk::info::Present presentInfo =
      vk::info::Present()
          .addWaitSemaphore(*frames[currentFrame].renderFinished)
          .addSwapchain(*swapchain)
          .setImageIndex(imageIndex);

  presentQueue.present(presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::recordCommandBuffer(CommandBuffer &commandBuffer,
                              uint32_t imageIndex) {
  auto encoder = commandBuffer.begin();

  vk::info::RenderPassBegin renderPassInfo(
      *renderPass, *framebuffers[imageIndex],
      VkRect2D{.offset = {0, 0}, .extent = swapchain.getExtent()});

  renderPassInfo.addClearValue({.color = {{0.0f, 0.0f, 0.0f, 1.0f}}});

  auto pass = encoder.beginRenderPass(renderPassInfo);

  pass.bindPipeline(pipeline);

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain.getExtent().width),
      .height = static_cast<float>(swapchain.getExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};

  VkRect2D scissor = {.offset = {0, 0}, .extent = swapchain.getExtent()};

  pass.setViewport(viewport);
  pass.setScissor(scissor);

  pass.bindVertexBuffer(0, vertexBuffer.vertexBuffer);

  pass.draw(3);

  pass.end();

  encoder.end();
}

App::~App() {
  if (moveGuard.isMoved()) {
    return;
  }

  device.waitIdle();

  glfwTerminate();
}
