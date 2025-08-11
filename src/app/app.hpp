#pragma once

#include <expected>
#include <string>

#include "vulkan/vulkan_raii.hpp"
#include <GLFW/glfw3.h>
#include <memory>

const int MAX_FRAMES_IN_FLIGHT = 2;

class App {
  std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window;
  vk::raii::Context context;
  vk::raii::Instance instance;

public:
  App(const App &) = delete;
  App(App &&) = default;

  static auto create() -> std::expected<App, std::string>;

  void poll() const { glfwPollEvents(); }

  auto shouldClose() const -> bool {
    return glfwWindowShouldClose(window.get());
  }

  int currentFrame = 0;

  App(std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)> window,
      vk::raii::Context context, vk::raii::Instance instance)
      : window(std::move(window)), context(std::move(context)),
        instance(std::move(instance)) {}
};
