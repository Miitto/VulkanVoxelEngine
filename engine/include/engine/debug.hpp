#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace engine {
VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
              [[maybe_unused]] void *pUserData) noexcept;

std::expected<vk::raii::DebugUtilsMessengerEXT, std::string>
makeDebugMessenger(vk::raii::Instance &instance,
                   void *pUserData = nullptr) noexcept;
} // namespace engine
