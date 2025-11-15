#pragma once

#include <algorithm>
#include <span>
#include <spdlog/fwd.h>
#include <vulkan/vulkan_raii.hpp>

namespace vkh {

void printExtensions(vk::raii::Context &context,
                     spdlog::level::level_enum logLevel) noexcept;

std::vector<const char *>
checkExtensions(vk::raii::Context &context,
                std::span<const char *> extensions) noexcept;
std::vector<const char *> checkLayers(vk::raii::Context &context,
                                      std::span<const char *> layers) noexcept;
} // namespace vkh
