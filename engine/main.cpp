#include "app.h"
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
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

    if (!update())
      return;
    if (!render())
      return;
  }
  std::println("App closed.");
}

bool App::update() { return true; }

bool App::render() {
  frames[currentFrame].inFlight.wait();
  auto [swpachainState, imageIndex] =
      swapchain.getNextImage(*frames[currentFrame].imageAvailable);

  switch (swpachainState) {
  case VK_SUBOPTIMAL_KHR:
  case VK_ERROR_OUT_OF_DATE_KHR:
    std::println("TODO: Recreate swapchain");
    return false;
  case VK_SUCCESS:
    break;
  default: {
    std::cerr << "Failed to acquire swapchain image." << std::endl;
    return false;
  }
  }
  frames[currentFrame].inFlight.reset();

  auto &uniform = uniforms.objects[currentFrame];

  CameraMatrices matrices{
      .model = glm::mat4(1.0f),
      .view = glm::lookAt(glm::vec3(3., 3., 3.), {0., 0., 0.}, {0., 0., 1.}),
      .projection =
          glm::perspective(glm::radians(90.0f),
                           static_cast<float>(swapchain.getExtent().width) /
                               swapchain.getExtent().height,
                           0.1f, 100.0f),
  };
  matrices.projection[1][1] *= -1; // Vulkan uses a different Y-axis

  if (!uniform.bufferMapping.write(&matrices, sizeof(matrices)))
    return false;

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
    return false;
  }

  vk::info::Present presentInfo =
      vk::info::Present()
          .addWaitSemaphore(*frames[currentFrame].renderFinished)
          .addSwapchain(*swapchain)
          .setImageIndex(imageIndex);

  presentQueue.present(presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return true;
}

void App::recordCommandBuffer(vk::CommandBuffer &commandBuffer,
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
  pass.bindIndexBuffer(vertexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

  pass.bindDescriptorSet(uniforms.objects[currentFrame].descriptorSet);

  pass.drawIndexed(6);

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
