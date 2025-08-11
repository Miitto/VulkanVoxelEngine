#include "app/app.hpp"
#include <expected>

#include <GLFW/glfw3.h>
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
  auto instance =
      vk::raii::Instance(vk::raii::Context(), vk::InstanceCreateInfo());

  Logger::trace("Creating App");
  App app(std::move(window), std::move(context), std::move(instance));

  return app;
}
