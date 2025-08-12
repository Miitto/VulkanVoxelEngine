#include "app/app.hpp"
#include "camera.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <optional>

#include "logger.hpp"

#include <engine/util/window_manager.hpp>
#include <vulkan/vulkan_raii.hpp>

class Program {
  App app;

  Program(App &app) : app(std::move(app)) {}

public:
  static auto create() -> std::optional<Program> {
    auto app_opt = App::create();
    if (!app_opt.has_value()) {
      Logger::critical("Failed to create App.");
      return std::nullopt;
    }

    return Program(app_opt.value());
  }

  void run() {
    Logger::info("Starting application...");

    while (!app.shouldClose()) {
      app.poll();

      if (!update()) {
        Logger::error("Update failed, exiting...");
        break;
      }

      if (!render()) {
        Logger::error("Render failed, exiting...");
        break;
      }

      // Simulate frame completion
      app.currentFrame = (app.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
  }

  auto update() -> bool { return true; }

  auto render() -> bool { return true; }
};

auto main() -> int {
  Logger::init();
  auto windowManager = WindowManager();
  windowManager.setResizable(false);

  auto app = Program::create();

  if (!app.has_value()) {
    Logger::critical("Failed to create Program.");
    return EXIT_FAILURE;
  }

  app->run();

  glfwTerminate();

  Logger::info("Application terminated successfully.");
  return EXIT_SUCCESS;
}
