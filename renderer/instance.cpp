#include "instance.h"
#include "surface.h"

namespace vk {
std::optional<Instance> Instance::create(vk::info::InstanceCreate &createInfo) {
  VkInstance instance;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    return std::nullopt;
  }
  return Instance(instance);
}

std::optional<Surface> Instance::createSurface(Window &window) {
  return Surface::create(*this, window);
}
} // namespace vk
