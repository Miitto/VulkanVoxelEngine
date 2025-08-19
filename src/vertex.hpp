#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static auto getBindingDescription() noexcept
      -> const vk::VertexInputBindingDescription;
  static auto getAttributeDescriptions() noexcept
      -> const std::array<vk::VertexInputAttributeDescription, 2>;
};
