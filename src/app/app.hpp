#pragma once

#include <expected>
#include <string>

#include "vulkan/vulkan_raii.hpp"
#include <GLFW/glfw3.h>
#include <memory>

const int MAX_FRAMES_IN_FLIGHT = 2;

class App {
public:
  struct Queue {
    uint32_t index;
    std::shared_ptr<vk::raii::Queue> queue;
  };

  struct Queues {
    Queue graphicsQueue;
    Queue presentQueue;
  };

  struct SwapchainConfig {
    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentMode;
    vk::Extent2D extent;
    uint32_t minImageCount;
    uint32_t imageCount;
  };

  struct Swapchain {
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;
  };

private:
  std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window;
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface;

  vk::raii::Device device;
  Queues queues;
  SwapchainConfig swapchainConfig;
  Swapchain swapchain;

public:
  App(const App &) = delete;
  App(App &&) = default;

  static auto create() -> std::expected<App, std::string>;

  void poll() const { glfwPollEvents(); }

  [[nodiscard]]
  auto shouldClose() const -> bool {
    return glfwWindowShouldClose(window.get());
  }

  int currentFrame = 0;

  App(std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> &window,
      vk::raii::Context &context, vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface,
      vk::raii::Device &device, Queues &queues,
      SwapchainConfig &swapchainConfig, Swapchain &swapchain)
      : window(std::move(window)), context(std::move(context)),
        instance(std::move(instance)), surface(std::move(surface)), device(std::move(device)),
        queues(std::move(queues)), swapchainConfig(swapchainConfig),
        swapchain(std::move(swapchain)) {}

  void setDebugMessenger(vk::raii::DebugUtilsMessengerEXT &&dMessenger) {
    debugMessenger = std::move(dMessenger);
  }
};
