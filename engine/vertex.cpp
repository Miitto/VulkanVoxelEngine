#include "vertex.h"
#include <array>

const vk::VertexInputBindingDescription Vertex::getBindingDescription() {
  return VkVertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(Vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };
}

const std::array<vk::VertexInputAttributeDescription, 2>
Vertex::getAttributeDescriptions() {
  return std::array<vk::VertexInputAttributeDescription, 2>{
      {{{.location = 0,
         .binding = 0,
         .format = VK_FORMAT_R32G32B32_SFLOAT,
         .offset = offsetof(Vertex, position)}},
       {
           {.location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)},
       }}};
}
