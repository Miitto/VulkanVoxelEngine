#pragma once

#include "logger.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace engine {
VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
              [[maybe_unused]] void *pUserData) noexcept;
auto makeDebugMessenger(vk::raii::Instance &instance,
                        void *pUserData = nullptr) noexcept;
} // namespace engine
