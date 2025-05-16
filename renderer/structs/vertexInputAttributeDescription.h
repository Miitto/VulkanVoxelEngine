#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {
class VertexInputAttributeDescription
    : public VkVertexInputAttributeDescription {
public:
  VertexInputAttributeDescription(uint32_t binding = 0, uint32_t location = 0,
                                  VkFormat format = VK_FORMAT_UNDEFINED,
                                  uint32_t offset = 0)
      : VkVertexInputAttributeDescription{
            .location = location,
            .binding = binding,
            .format = format,
            .offset = offset,
        } {}
  VertexInputAttributeDescription(const VkVertexInputAttributeDescription &desc)
      : VkVertexInputAttributeDescription(desc) {}
};
} // namespace vk
