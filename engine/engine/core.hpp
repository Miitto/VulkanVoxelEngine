#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan_raii.hpp>

namespace engine::rendering {
struct WindowAttribs {
  const int32_t width;
  const int32_t height;
  const char *title;
};
struct InstanceAdditions {
  const std::span<const char *const> extraExtensions;
  const std::span<const char *const> extraLayers;
};
class Core {
  using WinPtr = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>;

  WinPtr window;
  vk::raii::Context context;
  vk::raii::Instance instance;
  vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
  vk::raii::SurfaceKHR surface;

  Core(WinPtr &window, vk::raii::Context &context, vk::raii::Instance &instance,
       vk::raii::SurfaceKHR &surface) noexcept
      : window(std::move(window)), context(std::move(context)),
        instance(std::move(instance)), surface(std::move(surface)) {}

  inline auto
  setDebugMessenger(vk::raii::DebugUtilsMessengerEXT &debugMessenger) noexcept
      -> void {
    this->debugMessenger = std::move(debugMessenger);
  }

public:
  static auto create(const WindowAttribs windowAttribs,
                     const InstanceAdditions &instanceAdditions = {},
                     const bool enableValidationLayers = false)
      -> std::expected<Core, std::string>;

  [[nodiscard]] inline auto getWindow() const noexcept -> const WinPtr & {
    return window;
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
