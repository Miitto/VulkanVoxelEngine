#pragma once

#include <GLFW/glfw3.h>
#include <expected>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace engine::vulkan {

struct SwapchainConfig {
  vk::SurfaceFormatKHR format;
  vk::PresentModeKHR presentMode;
  vk::Extent2D extent;
  uint32_t minImageCount;
  uint32_t imageCount;
};

auto chooseSwapSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> &availableFormats)
    -> vk::SurfaceFormatKHR;
auto chooseSwapPresentMode(
    const std::vector<vk::PresentModeKHR> &availablePresentModes)
    -> vk::PresentModeKHR;

auto chooseSwapExtent(GLFWwindow *window,
                      const vk::SurfaceCapabilitiesKHR &capabilities)
    -> vk::Extent2D;

auto minImageCount(const vk::SurfaceCapabilitiesKHR &capabilities,
                   uint32_t desired) -> uint32_t;

auto desiredImageCount(const vk::SurfaceCapabilitiesKHR &capabilities)
    -> uint32_t;

struct SwapchainQueues {
  uint32_t graphicsQueueIndex;
  uint32_t presentQueueIndex;
};

class Swapchain {
  vk::raii::SwapchainKHR swapchain;
  std::vector<vk::Image> imgs;
  std::vector<vk::raii::ImageView> imageViews;

  Swapchain(vk::raii::SwapchainKHR &swapchain, std::vector<vk::Image> &images,
            std::vector<vk::raii::ImageView> &imageViews)
      : swapchain(std::move(swapchain)), imgs(std::move(images)),
        imageViews(std::move(imageViews)) {}

public:
  static auto
  create(const vk::raii::Device &device, const SwapchainConfig &swapchainConfig,
         const vk::raii::PhysicalDevice &physicalDevice,
         const vk::raii::SurfaceKHR &surface, const SwapchainQueues &queues,
         std::optional<vk::raii::SwapchainKHR *> oldSwapchain)
      -> std::expected<Swapchain, std::string>;

  [[nodiscard]] auto images() const -> const std::vector<vk::Image> & {
    return imgs;
  }

  [[nodiscard]] auto views() const -> const std::vector<vk::raii::ImageView> & {
    return imageViews;
  }

  [[nodiscard]] auto nImage(const size_t index) const -> const vk::Image & {
    return imgs[index];
  }

  [[nodiscard]] auto nView(const size_t index) const
      -> const vk::raii::ImageView & {
    return imageViews[index];
  }

  [[nodiscard]] auto getSwapchain() const -> const vk::raii::SwapchainKHR & {
    return swapchain;
  }

  auto operator*() -> vk::raii::SwapchainKHR & { return swapchain; }
  auto operator*() const -> const vk::raii::SwapchainKHR & { return swapchain; }

  auto operator->() -> vk::raii::SwapchainKHR * { return &swapchain; }
  auto operator->() const -> const vk::raii::SwapchainKHR * {
    return &swapchain;
  }

  auto operator[](const size_t index) -> vk::raii::ImageView & {
    return imageViews[index];
  }
  auto operator[](const size_t index) const -> const vk::raii::ImageView & {
    return imageViews[index];
  }

  enum class State : uint8_t { Ok, Suboptimal, OutOfDate };

  [[nodiscard]] auto getNextImage(const vk::raii::Device &device,
                                  const vk::raii::Fence &waitFence,
                                  const vk::raii::Semaphore &signalSem) const
      -> std::expected<std::pair<uint32_t, State>, std::string>;
};
} // namespace engine::vulkan
