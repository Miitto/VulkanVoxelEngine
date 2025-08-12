#pragma once

#include "logger.hpp"
#include <vulkan/vulkan_raii.hpp>

namespace engine {
VKAPI_ATTR vk::Bool32 VKAPI_CALL
debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              vk::DebugUtilsMessageTypeFlagsEXT messageType,
              const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
              [[maybe_unused]] void *pUserData) {
  switch (messageSeverity) {
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
    Logger::trace("Vulkan ({}): {}", to_string(messageType),
                  pCallbackData->pMessage);
    break;
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
    Logger::info("Vulkan ({}): {}", to_string(messageType),
                 pCallbackData->pMessage);
    break;
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
    Logger::warn("Vulkan ({}): {}", to_string(messageType),
                 pCallbackData->pMessage);
    break;
  case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
    Logger::error("Vulkan ({}): {}", to_string(messageType),
                  pCallbackData->pMessage);
    break;
  default:
    Logger::debug("Vulkan ({}): {}", to_string(messageType),
                  pCallbackData->pMessage);
  }
  return VK_FALSE;
}

auto makeDebugMessenger(vk::raii::Instance &instance,
                        void *pUserData = nullptr) {

  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .messageSeverity = severityFlags,
      .messageType = messageTypeFlags,
      .pfnUserCallback = &debugCallback,
      .pUserData = pUserData,
  };
  return instance.createDebugUtilsMessengerEXT(
      debugUtilsMessengerCreateInfoEXT);
}
} // namespace engine
