#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
class VertexInputBindingDescription : public VkVertexInputBindingDescription {
public:
  VertexInputBindingDescription(
      uint32_t binding = 0, uint32_t stride = 0,
      VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX)
      : VkVertexInputBindingDescription{
            .binding = binding,
            .stride = stride,
            .inputRate = inputRate,
        } {}
  VertexInputBindingDescription(const VkVertexInputBindingDescription &desc)
      : VkVertexInputBindingDescription(desc) {}

  VertexInputBindingDescription &setBinding(uint32_t bind) {
    binding = bind;

    return *this;
  }

  VertexInputBindingDescription &setStride(uint32_t str) {
    stride = str;

    return *this;
  }

  VertexInputBindingDescription &setInputRate(VkVertexInputRate rate) {
    inputRate = rate;

    return *this;
  }
};
} // namespace vk
