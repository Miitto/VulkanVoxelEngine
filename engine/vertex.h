#include "structs/vertexInputAttributeDescription.h"
#include "structs/vertexInputBindingDescription.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static const vk::VertexInputBindingDescription getBindingDescription();
  static const std::array<vk::VertexInputAttributeDescription, 2>
  getAttributeDescriptions();
};
