#pragma once

#include "validators.hpp"
#include <expected>
#include <span>
#include <vulkan/vulkan_raii.hpp>

namespace vkh {
auto createInstance(
    vk::raii::Context &context, const char *appName,
    const bool enableValidationLayers,
    const std::span<const char *const> extraExtensions = {},
    const std::span<const char *const> extraLayers = {}) noexcept
    -> std::expected<vk::raii::Instance, std::string>;
} // namespace vkh
