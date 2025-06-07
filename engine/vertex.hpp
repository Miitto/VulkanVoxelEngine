#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <vk/structs/vertexInputAttributeDescription.hpp>
#include <vk/structs/vertexInputBindingDescription.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static auto getBindingDescription()
      -> const vk::VertexInputBindingDescription;
  static auto getAttributeDescriptions()
      -> const std::array<vk::VertexInputAttributeDescription, 2>;
};
