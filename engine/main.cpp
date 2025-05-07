#include "lib.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "Vulkan Application Starting..." << std::endl;
  auto app = App::create();
  if (!app.has_value()) {
    std::cout << "Failed to create application." << std::endl;
    return EXIT_FAILURE;
  }

  try {
    app->run();
  } catch (const std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Unknown exception occurred." << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Application terminated successfully." << std::endl;
  return EXIT_SUCCESS;
}
