#pragma once

#include "engine/window.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>

namespace engine::rendering {

struct InstanceAdditions {
  const std::span<const char *const> extraExtensions;
  const std::span<const char *const> extraLayers;
};
class Core {
  engine::Window window;
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface;

  Core(engine::Window &window, vk::raii::Context &context,
       vk::raii::Instance &instance, vk::raii::SurfaceKHR &surface) noexcept
      : window(std::move(window)), context(std::move(context)),
        instance(std::move(instance)), surface(std::move(surface)) {}

  inline auto
  setDebugMessenger(vk::raii::DebugUtilsMessengerEXT &debugMessenger) noexcept
      -> void {
    this->debugMessenger = std::move(debugMessenger);
  }

public:
  static auto create(const engine::Window::Attribs windowAttribs,
                     const InstanceAdditions &instanceAdditions = {},
                     const bool enableValidationLayers = false) noexcept
      -> std::expected<Core, std::string>;

  [[nodiscard]] inline auto getWindow() noexcept -> Window & { return window; }

  [[nodiscard]] inline auto getWindow() const noexcept -> const Window & {
    return window;
  }

  [[nodiscard]] inline auto getContext() noexcept -> vk::raii::Context & {
    return context;
  }

  [[nodiscard]] inline auto getInstance() noexcept -> vk::raii::Instance & {
    return instance;
  }

  [[nodiscard]] inline auto getSurface() noexcept -> vk::raii::SurfaceKHR & {
    return surface;
  }

  [[nodiscard]] inline auto getContext() const noexcept
      -> const vk::raii::Context & {
    return context;
  }

  [[nodiscard]] inline auto getInstance() const noexcept
      -> const vk::raii::Instance & {
    return instance;
  }

  [[nodiscard]] inline auto getSurface() const noexcept
      -> const vk::raii::SurfaceKHR & {
    return surface;
  }
};
} // namespace engine::rendering
