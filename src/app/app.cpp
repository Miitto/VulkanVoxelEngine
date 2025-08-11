#include "app/app.hpp"
#include <expected>

#include "util/error.hpp"
#include <GLFW/glfw3.h>
#include <format>
#include <memory>

#include "logger.hpp"

auto App::create() -> std::expected<App, std::string> {
  Logger::info("Creating GLFW window...");
  auto win_ptr = glfwCreateWindow(800, 600, "Vulkan App", nullptr, nullptr);
  auto window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      win_ptr, [](GLFWwindow *window) {
        Logger::info("Destroying window");
        glfwDestroyWindow(window);
      });

  Logger::trace("Creating Context");
  auto context = vk::raii::Context();

  Logger::trace("Creating Instance");
  auto appInfo = vk::ApplicationInfo{
      .pApplicationName = "Vulkan App",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_4,
  };

  auto glfwExtCount = 0u;
  auto glfwRequiredExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtCount);

  auto extensions = std::vector<const char *>{};
  extensions.reserve(glfwExtCount);
  for (auto i = 0u; i < glfwExtCount; ++i) {
    extensions.push_back(glfwRequiredExtensions[i]);
  }

  auto layerNames = std::vector<const char *>{};

#ifndef NDEBUG
  layerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif

  auto iCreateInfo = vk::InstanceCreateInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(layerNames.size()),
      .ppEnabledLayerNames = layerNames.data(),
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()};
  auto instance_res = context.createInstance(iCreateInfo);

  if (!instance_res) {
    // TODO: Make a formatter for Result
    Logger::error("Failed to create Vulkan instance: {}",
                  Result(instance_res.error()).toString());
    return std::unexpected("Failed to create Vulkan instnace");
  }

  auto instance = std::move(instance_res.value());

  Logger::trace("Creating App");
  App app(std::move(window), std::move(context), std::move(instance));

  return app;
}
