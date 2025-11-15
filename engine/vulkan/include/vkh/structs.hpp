#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace vkh {
struct Queue {
  uint32_t index;
  std::shared_ptr<vk::raii::Queue> queue;
};
} // namespace vkh