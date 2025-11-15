#include "app/app.hpp"

#include "camera.hpp"
#include "glm/geometric.hpp"
#include "logger.hpp"
#include "pipelines/pipelines.hpp"
#include "vertex.hpp"
#include "vkh/memorySelector.hpp"
#include <engine/util/macros.hpp>
#include <engine/window_manager.hpp>
#include <utility>
#include <vkh/shader.hpp>
#include <vulkan/vulkan_raii.hpp>

auto main() noexcept -> int {
  Logger::init();
  auto windowManager = WindowManager();

  auto app = App::create();

  if (!app.has_value()) {
    Logger::critical("Failed to create Program.");
    return EXIT_FAILURE;
  }

  engine::run(*app);

  Logger::info("Application terminated successfully.");
  return EXIT_SUCCESS;
}
