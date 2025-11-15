#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace engine {
void transitionImageLayout(const vk::raii::CommandBuffer &commandBuffer,
                           const vk::Image &image, vk::ImageLayout oldLayout,
                           vk::ImageLayout newLayout,
                           vk::AccessFlags2 srcAccessMask,
                           vk::AccessFlags2 dstAccessMask,
                           vk::PipelineStageFlags2 srcStageMask,
                           vk::PipelineStageFlags2 dstStageMask) noexcept;
} // namespace engine
