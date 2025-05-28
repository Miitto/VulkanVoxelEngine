module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

import :info.instanceCreate;

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
