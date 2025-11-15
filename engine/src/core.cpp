#include "engine/core.hpp"

#include "logger.hpp"
#include "vkh/instance.hpp"

namespace engine::rendering {

auto Core::create(const engine::Window::Attribs windowAttribs,
                  const InstanceAdditions &instanceAdditions,
                  const bool enableValidationLayers) noexcept
    -> std::expected<Core, std::string> {
  Logger::info("Creating GLFW window...");
  auto window = engine::Window(windowAttribs);

  Logger::trace("Creating Context");
  auto context = vk::raii::Context();

  auto instance_res = vkh::createInstance(
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
