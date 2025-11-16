#include "app/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>

#include "logger.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/setup.hpp>
#include <engine/util/macros.hpp>
#include <vkh/physicalDeviceSelector.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>

namespace {

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *const WINDOW_TITLE = "Vulkan App IGNORE";

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::array<const char *, 3> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName};
} // namespace

std::expected<App, std::string> App::create() noexcept {
  EG_MAKE(core,
          engine::rendering::Core::create(
              engine::Window::Attribs{
                  .width = WINDOW_WIDTH,
                  .height = WINDOW_HEIGHT,
                  .title = WINDOW_TITLE,
              },
              {}, enableValidationLayers),
          "Failed to create core");

  EG_MAKE(physicalDevice,
          engine::setup::selectPhysicalDevice(
              core.getInstance(),
              [](vkh::PhysicalDeviceSelector &selector)
                  -> std::optional<std::string> {
                selector.requireExtensions(requiredDeviceExtensions);
                selector.requireVersion(1, 4, 0);
                selector.requireQueueFamily(vk::QueueFlagBits::eGraphics);

                selector.scoreDevices(
                    [](const vkh::PhysicalDeviceSelector::DeviceSpecs &spec) {
                      uint32_t score = 0;
                      if (spec.properties.deviceType ==
                          vk::PhysicalDeviceType::eDiscreteGpu) {
                        score += 1000;
                      }

                      return score;
                    });
                return std::nullopt;
              }),
          "Failed to select physical device");

  EG_MAKE(coreQueuesIndices,
          engine::setup::findCoreQueues(physicalDevice, core.getSurface()),
          "Failed to find core queue families");

  std::vector<engine::setup::QueueCreateInfo> queueCreateInfos;
  queueCreateInfos.push_back(
      {.familyIndex = coreQueuesIndices.graphics, .priority = 1.0f});
  if (coreQueuesIndices.present != coreQueuesIndices.graphics)
    queueCreateInfos.push_back(
        {.familyIndex = coreQueuesIndices.present, .priority = 1.0f});

  EG_MAKE(device,
          engine::setup::createLogicalDevice(
              physicalDevice, queueCreateInfos,
              engine::setup::ENGINE_DEVICE_EXTENSIONS,
              requiredDeviceExtensions),
          "Failed to create logical device");

  EG_MAKE(queues,
          engine::setup::retrieveQueues(
              device, {coreQueuesIndices.graphics, coreQueuesIndices.present}),
          "Failed to retrieve queues");

  engine::App::Queues coreQueues{
      .graphics = queues[0],
      .present = queues[1],
  };

  vma::AllocatorCreateInfo allocCreateInfo{
      .flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress,
      .physicalDevice = physicalDevice,
      .device = device,
      .instance = core.getInstance(),
  };

  VMA_MAKE(allocator, vma::createAllocator(allocCreateInfo),
           "Failed to make allocator");

  EG_MAKE(swapchain,
          engine::setup::createSwapchain(physicalDevice, device,
                                         core.getWindow(), core.getSurface(),
                                         coreQueuesIndices, std::nullopt),
          "Failed to create swapchain");

  EG_MAKE(renderImage,
          engine::setup::createRenderImage(device, allocator,
                                           swapchain.config(),
                                           vk::Format::eR16G16B16A16Sfloat),
          "Failed to create render image");

  VK_MAKE(commandPool,
          device.createCommandPool(vk::CommandPoolCreateInfo{
              .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
              .queueFamilyIndex = coreQueuesIndices.graphics}),
          "Failed to create command pool");

  EG_MAKE(sync1, engine::setup::createSyncObjects(device),
          "Failed to create sync objects");
  EG_MAKE(sync2, engine::setup::createSyncObjects(device),
          "Failed to create sync objects");

  std::array<engine::SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects = {
      std::move(sync1), std::move(sync2)};

  engine::Input::instance().setupWindow(core.getWindow());

  EG_MAKE(imGuiObjects,
          engine::setup::setupImGui(core.getWindow().get(), core.getInstance(),
                                    device, physicalDevice, coreQueues.graphics,
                                    swapchain.config().format.format),
          "Failed to setup ImGui Vulkan objects");

  vk::CommandBufferAllocateInfo commandBufferAllocInfo{
      .commandPool = *commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};

  VK_MAKE(commandBuffersV,
          device.allocateCommandBuffers(commandBufferAllocInfo),
          "Failed to allocate command buffers");

  std::array<vk::raii::CommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers{
      std::move(commandBuffersV[0]), std::move(commandBuffersV[1])};

  using pipelines::Mesh;

  std::array<pipelines::Mesh::Vertex, 4> vertices = {
      Mesh::Vertex{.position = {-0.5f, -0.5f, 0.0f},
                   .uvX = -0.5f,
                   .normal = engine::FORWARD,
                   .uvY = 0.5f,
                   .color = {1.0f, 0.0f, 0.0f, 1.0f}},
      Mesh::Vertex{.position = {0.5f, -0.5f, 0.0f},
                   .uvX = 0.5f,
                   .normal = engine::FORWARD,
                   .uvY = -0.5f,
                   .color = {0.0f, 1.0f, 0.0f, 1.0f}},
      Mesh::Vertex{.position = {-0.5f, 0.5f, 0.0f},
                   .uvX = -0.5f,
                   .normal = engine::FORWARD,
                   .uvY = 0.5f,
                   .color = {0.0f, 0.0f, 1.0f, 1.0f}},
      Mesh::Vertex{.position = {0.5f, 0.5f, 0.0f},
                   .uvX = 0.5f,
                   .normal = engine::FORWARD,
                   .uvY = 0.5f,
                   .color = {1.0f, 1.0f, 1.0f, 1.0f}}};

  vk::BufferCreateInfo vertexBufferInfo{
      .size = sizeof(Mesh::Vertex) * vertices.size(),
      .usage = vk::BufferUsageFlagBits::eTransferDst |
               vk::BufferUsageFlagBits::eStorageBuffer |
               vk::BufferUsageFlagBits::eShaderDeviceAddress,
      .sharingMode = vk::SharingMode::eExclusive};

  vma::AllocationCreateInfo vertexAllocInfo{
      .usage = vma::MemoryUsage::eGpuOnly,
  };

  vkh::AllocatedBuffer vBuffer;
  VMA_MAKE(vertexBufferPair,
           allocator.createBuffer(vertexBufferInfo, vertexAllocInfo,
                                  &vBuffer.allocInfo),
           "Failed to create vertex buffer");
  vBuffer.buffer = vertexBufferPair.first;
  vBuffer.alloc = vertexBufferPair.second;

  vkh::AllocatedBuffer stagingBuffer;
  VMA_MAKE(
      stagingBufferPair,
      allocator.createBuffer(
          vk::BufferCreateInfo{.size = vertexBufferInfo.size,
                               .usage = vk::BufferUsageFlagBits::eTransferSrc,
                               .sharingMode = vk::SharingMode::eExclusive},
          vma::AllocationCreateInfo{
              .flags = vma::AllocationCreateFlagBits::eMapped,
              .usage = vma::MemoryUsage::eCpuOnly,
          },
          &stagingBuffer.allocInfo),
      "Failed to create staging buffer");
  stagingBuffer.buffer = stagingBufferPair.first;
  stagingBuffer.alloc = stagingBufferPair.second;

  memcpy(stagingBuffer.allocInfo.pMappedData, vertices.data(),
         vertexBufferInfo.size);

  {
    auto &cmdBuf = commandBuffers[0];

    cmdBuf.begin(vk::CommandBufferBeginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    vk::BufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = vertexBufferInfo.size,
    };

    cmdBuf.copyBuffer(stagingBuffer.buffer, vBuffer.buffer, copyRegion);

    cmdBuf.end();

    vk::CommandBufferSubmitInfo submitInfo{.commandBuffer = cmdBuf};
    vk::SubmitInfo2 submitInfos{
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &submitInfo,
    };

    VK_MAKE(waitFence, device.createFence(vk::FenceCreateInfo{}),
            "Failed to create fence");

    coreQueues.graphics.queue->submit2(submitInfos, waitFence);

    auto res = device.waitForFences({waitFence}, VK_TRUE, UINT64_MAX);
    if (res != vk::Result::eSuccess) {
      return std::unexpected(
          "Failed to wait for fence after vertex buffer copy");
    }

    stagingBuffer.destroy(allocator);
  }

  vk::DescriptorPoolSize poolSize{.type = vk::DescriptorType::eUniformBuffer,
                                  .descriptorCount = MAX_FRAMES_IN_FLIGHT};
  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = MAX_FRAMES_IN_FLIGHT,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize};
  VK_MAKE(cameraDescriptorPool, device.createDescriptorPool(poolInfo),
          "Failed to create camera descriptor pool");

  EG_MAKE(cameraDescriptorLayout, PerspectiveCamera::descriptorLayout(device),
          "Failed to create camera descriptor layout");

  EG_MAKE(cameraBuffers,
          PerspectiveCamera::createBuffers(
              device, allocator, cameraDescriptorPool, cameraDescriptorLayout),
          "Failed to create uniform buffers");

  EG_MAKE(basicVertexPipeline,
          pipelines::Mesh::create(device, renderImage.format,
                                  pipelines::Mesh::DescriptorLayouts{
                                      .camera = cameraDescriptorLayout}),
          "Failed to create basic vertex pipeline");

  constexpr glm::vec3 CAMERA_START_POS = {0.0f, 0.0f, 2.0f};
  constexpr float CAMERA_START_FOV = glm::radians(90.0f);
  constexpr float CAMERA_NEAR_PLANE = 0.1f;

  PerspectiveCamera camera(
      CAMERA_START_POS, {},
      engine::cameras::Perspective::Params{
          .fov = CAMERA_START_FOV,
          .aspectRatio = static_cast<float>(swapchain.config().extent.width) /
                         static_cast<float>(swapchain.config().extent.height),
          .nearPlane = CAMERA_NEAR_PLANE,
      });

  CameraObjects camObjs{.pool = std::move(cameraDescriptorPool),
                        .buffers = std::move(cameraBuffers),
                        .camera = std::move(camera)};

  return App(std::move(core), std::move(physicalDevice), std::move(device),
             allocator, std::move(coreQueues), std::move(swapchain),
             std::move(renderImage), std::move(commandPool),
             std::move(syncObjects), std::move(imGuiObjects),
             std::move(commandBuffers), std::move(camObjs),
             std::move(basicVertexPipeline), vBuffer);
}
