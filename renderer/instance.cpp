#include "instance.h"
#include "surface.h"

std::optional<Instance> Instance::create(VkInstanceCreateInfo &createInfo) {
  VkInstance instance;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    return std::nullopt;
  }
  return Instance(instance);
}

std::optional<Surface> Instance::createSurface(Window &window) {
  return Surface::create(*this, window);
}
