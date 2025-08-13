#include "core.hpp"

#include "engine/vulkan/createInstance.hpp"
#include "logger.hpp"

namespace engine::rendering {

auto Core::create(const WindowAttribs windowAttribs,
                  const InstanceAdditions &instanceAdditions,
                  const bool enableValidationLayers)
    -> std::expected<Core, std::string> {
  Logger::info("Creating GLFW window...");
  auto win_ptr = glfwCreateWindow(windowAttribs.width, windowAttribs.height,
                                  windowAttribs.title, nullptr, nullptr);
  auto window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      win_ptr, [](GLFWwindow *window) {
        Logger::info("Destroying window");
        glfwDestroyWindow(window);
      });

  Logger::trace("Creating Context");
  auto context = vk::raii::Context();

  auto instance_res = engine::vulkan::createInstance(
      context, "Voxel Engine", enableValidationLayers,
      instanceAdditions.extraExtensions, instanceAdditions.extraLayers);
  if (!instance_res) {
    return std::unexpected(instance_res.error());
  }
  auto instance = std::move(instance_res.value());

  VkSurfaceKHR rawSurface;
  if (glfwCreateWindowSurface(*instance, window.get(), nullptr, &rawSurface) !=
      VK_SUCCESS) {
    Logger::error("Failed to create window surface");
    return std::unexpected("Failed to create window surface");
  }

  auto surface = vk::raii::SurfaceKHR(instance, rawSurface);

  return Core(window, context, instance, surface);
}
} // namespace engine::rendering
