#pragma once

#include <optional>
#include <vulkan/vulkan_core.h>
class Pipeline {
public:
  static std::optional<Pipeline> create(VkPipelineCreateInfoKHR createInfo);
};
