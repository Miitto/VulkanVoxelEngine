#pragma once

#include <expected>
#include <string>

#include "logger.hpp"
#include "vkh/swapchain.hpp"
#include <GLFW/glfw3.h>
#include <engine/util/macros.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <engine/image.hpp>

#include "engine/structs.hpp"
#include "vkh/physicalDeviceSelector.hpp"
#include <vkh/structs.hpp>

namespace engine::setup {
std::expected<vk::raii::PhysicalDevice, std::string> selectPhysicalDevice(
    const vk::raii::Instance &instance,
    std::function<std::optional<std::string>(vkh::PhysicalDeviceSelector &)>);

struct CoreQueueFamilyIndices {
  uint32_t graphics;
  uint32_t present;

  [[nodiscard]] auto uniqueIndices() const noexcept -> std::vector<uint32_t> {
    if (graphics == present) {
      return {graphics};
    } else {
      return {graphics, present};
    }
  }
};

std::expected<CoreQueueFamilyIndices, std::string>
findCoreQueues(const vk::raii::PhysicalDevice &, const vk::raii::SurfaceKHR &);

struct QueueCreateInfo {
  uint32_t familyIndex;
  float priority = 1.0f;
};

const vk::StructureChain<
    vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
    vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
    ENGINE_DEVICE_EXTENSIONS = {
        {},
        {.shaderDrawParameters = true},
        {.bufferDeviceAddress = true},
        {.synchronization2 = true, .dynamicRendering = true},
        {.extendedDynamicState = true}};

template <typename... Ts>
std::expected<vk::raii::Device, std::string>
createLogicalDevice(const vk::raii::PhysicalDevice &physicalDevice,
                    std::span<QueueCreateInfo> indices,
                    const vk::StructureChain<Ts...> &chain,
                    std::span<const char *const> deviceExtensions) {
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
  queueCreateInfo.reserve(indices.size());
  for (const auto &index : indices) {
    std::array<float, 1> queuePriority = {index.priority};
    queueCreateInfo.push_back(
        vk::DeviceQueueCreateInfo{.queueFamilyIndex = index.familyIndex,
                                  .queueCount = 1,
                                  .pQueuePriorities = queuePriority.data()});
  }

  // Source - https://stackoverflow.com/a
  // Posted by Barry, modified by community. See post 'Timeline' for change
  // history Retrieved 2025-11-14, License - CC BY-SA 4.0
  using ChainStart = std::tuple_element_t<0, std::tuple<Ts...>>;

// Ingore missing designated initializers warning for clang
// The fields are deprecated so you'd get a warning with or without them
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
#endif
  auto deviceCreateInfo = vk::DeviceCreateInfo{
      .pNext = &chain.template get<ChainStart>(),
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
      .pQueueCreateInfos = queueCreateInfo.data(),
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};
#ifdef __clang__
#pragma clang diagnostic pop
#endif

  VK_MAKE(device, physicalDevice.createDevice(deviceCreateInfo),
          "Failed to create logical device");
  return std::move(device);
}

std::expected<std::vector<vkh::Queue>, std::string>
retrieveQueues(const vk::raii::Device &device,
               const std::vector<uint32_t> &indices);

std::expected<vkh::Swapchain, std::string>
createSwapchain(const vk::raii::PhysicalDevice &physicalDevice,
                const vk::raii::Device &device, const Window &window,
                const vk::raii::SurfaceKHR &surface,
                const CoreQueueFamilyIndices &queues,
                std::optional<vk::raii::SwapchainKHR *> oldSwapchain) noexcept;

std::expected<vkh::AllocatedImage, std::string>
createRenderImage(const vk::raii::Device &device, const vma::Allocator &alloc,
                  const vkh::SwapchainConfig &swapchainConfig,
                  vk::Format format);

auto createSyncObjects(const vk::raii::Device &device) noexcept
    -> std::expected<SyncObjects, std::string>;

std::expected<ImGuiVkObjects, std::string>
setupImGui(GLFWwindow *window, const vk::raii::Instance &instance,
           const vk::raii::Device &device,
           const vk::raii::PhysicalDevice &physicalDevice,
           const vkh::Queue &graphicsQueue, const vk::Format swapchainFormat);
} // namespace engine::setup
