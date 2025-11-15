#pragma once

#include "engine/input.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace engine {
struct SyncObjects {
  vk::raii::Semaphore presentCompleteSemaphore;
  vk::raii::Semaphore renderCompleteSemaphore;
  vk::raii::Fence drawingFence;
};

struct FrameData {
  float deltaTimeMs;
  const Input &input;
};

struct ImGuiVkObjects {
  vk::raii::DescriptorPool descriptorPool;
};

} // namespace engine
