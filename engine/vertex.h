#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  static const VkVertexInputBindingDescription getBindingDescription();
  static const std::array<VkVertexInputAttributeDescription, 2>
  getAttributeDescriptions();
};
