#include "app/app.hpp"

#include "logger.hpp"
#include <engine/window_manager.hpp>

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
