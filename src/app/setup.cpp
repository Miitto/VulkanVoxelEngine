#include "app/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "logger.hpp"
#include "vkh/queueFinder.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/setup.hpp>
#include <engine/util/macros.hpp>
#include <vertex.hpp>
#include <vkh/memorySelector.hpp>
#include <vkh/physicalDeviceSelector.hpp>
#include <vkh/pipeline.hpp>
#include <vkh/shader.hpp>

namespace {

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan App IGNORE";

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

  auto allocatorRes = vma::createAllocator(allocCreateInfo);
  if (allocatorRes.result != vk::Result::eSuccess) {
    Logger::error("Failed to create VMA allocator: {}",
                  vk::to_string(allocatorRes.result));
    return std::unexpected("Failed to create VMA allocator");
  }
  auto allocator = allocatorRes.value;

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

  std::array<Vertex, 4> vertices = {
      Vertex{.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
      Vertex{.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
      Vertex{.position = {-0.5f, 0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
      Vertex{.position = {0.5f, 0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}}};

  vk::BufferCreateInfo vertexBufferInfo{
      .size = sizeof(vertices[0]) * vertices.size(),
      .usage = vk::BufferUsageFlagBits::eVertexBuffer,
      .sharingMode = vk::SharingMode::eExclusive};

  VK_MAKE(vertexBuffer, device.createBuffer(vertexBufferInfo),
          "Failed to create vertex buffer");

  auto memSelector = vkh::MemorySelector(vertexBuffer, physicalDevice);

  EG_MAKE(vertexAllocInfo,
          memSelector.allocInfo(vk::MemoryPropertyFlagBits::eHostVisible |
                                vk::MemoryPropertyFlagBits::eHostCoherent),
          "Failed to get memory allocation info");

  VK_MAKE(vBufferMemory, device.allocateMemory(vertexAllocInfo),
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
  VK_MAKE(cameraDescriptorPool, device.createDescriptorPool(poolInfo),
          "Failed to create camera descriptor pool");

  EG_MAKE(cameraDescriptorLayout, PerspectiveCamera::descriptorLayout(device),
          "Failed to create camera descriptor layout");

  EG_MAKE(cameraBuffers,
          PerspectiveCamera::createBuffers(device, physicalDevice,
                                           cameraDescriptorPool,
                                           cameraDescriptorLayout),
          "Failed to create uniform buffers");

  EG_MAKE(
      basicVertexPipeline,
      pipelines::BasicVertex::create(device, renderImage.format,
                                     pipelines::BasicVertex::DescriptorLayouts{
                                         .camera = cameraDescriptorLayout}),
      "Failed to create basic vertex pipeline");

  PerspectiveCamera camera(
      {0.0f, 0.0f, 2.0f}, {},
      engine::cameras::Perspective::Params{
          .fov = glm::radians(90.0f),
          .aspectRatio = static_cast<float>(swapchain.config().extent.width) /
                         static_cast<float>(swapchain.config().extent.height),
          .nearPlane = 0.1f});

  CameraObjects camObjs{.pool = std::move(cameraDescriptorPool),
                        .buffers = std::move(cameraBuffers),
                        .camera = std::move(camera)};

  return App(std::move(core), std::move(physicalDevice), std::move(device),
             allocator, std::move(coreQueues), std::move(swapchain),
             std::move(renderImage), std::move(commandPool),
             std::move(syncObjects), std::move(imGuiObjects),
             std::move(commandBuffers), std::move(camObjs),
             std::move(basicVertexPipeline), std::move(vBufferMemory),
             std::move(vertexBuffer));
}
