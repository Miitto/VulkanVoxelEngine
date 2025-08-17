#include "vertex.hpp"
#include <array>

auto Vertex::getBindingDescription()
    -> const vk::VertexInputBindingDescription {
  return vk::VertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = vk::VertexInputRate::eVertex,
  };
}

auto Vertex::getAttributeDescriptions()
    -> const std::array<vk::VertexInputAttributeDescription, 2> {
  return std::array<vk::VertexInputAttributeDescription, 2>{{
      {.location = 0,
       .binding = 0,
       .format = vk::Format::eR32G32B32Sfloat,
       .offset = offsetof(Vertex, position)},
      {.location = 1,
       .binding = 0,
       .format = vk::Format::eR32G32B32Sfloat,
       .offset = offsetof(Vertex, color)},
  }};
}
