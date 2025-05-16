#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vk {
namespace info {
class PipelineVertexInputStateCreate
    : public VkPipelineVertexInputStateCreateInfo {

  std::vector<VkVertexInputBindingDescription> m_vertexBindingDescriptions;
  std::vector<VkVertexInputAttributeDescription> m_vertexAttributeDescriptions;

  void setupBindingDescriptions() {
    vertexBindingDescriptionCount =
        static_cast<uint32_t>(m_vertexBindingDescriptions.size());
    pVertexBindingDescriptions = m_vertexBindingDescriptions.data();
  }

  void setupAttributeDescriptions() {
    vertexAttributeDescriptionCount =
        static_cast<uint32_t>(m_vertexAttributeDescriptions.size());
    pVertexAttributeDescriptions = m_vertexAttributeDescriptions.data();
  }

public:
  PipelineVertexInputStateCreate()
      : VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr} {}

  PipelineVertexInputStateCreate &addBindingDescription(
      const VkVertexInputBindingDescription &bindingDescription) {
    m_vertexBindingDescriptions.push_back(bindingDescription);
    setupBindingDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &addBindingDescriptions(
      const std::span<VkVertexInputBindingDescription> &bindingDescriptions) {
    m_vertexBindingDescriptions.insert(m_vertexBindingDescriptions.end(),
                                       bindingDescriptions.begin(),
                                       bindingDescriptions.end());
    setupBindingDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &addAttributeDescription(
      const VkVertexInputAttributeDescription &attributeDescription) {
    m_vertexAttributeDescriptions.push_back(attributeDescription);
    setupAttributeDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &
  addAttributeDescriptions(const std::span<VkVertexInputAttributeDescription>
                               &attributeDescriptions) {
    m_vertexAttributeDescriptions.insert(m_vertexAttributeDescriptions.end(),
                                         attributeDescriptions.begin(),
                                         attributeDescriptions.end());
    setupAttributeDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate(const PipelineVertexInputStateCreate &other)
      : VkPipelineVertexInputStateCreateInfo{other},
        m_vertexBindingDescriptions(other.m_vertexBindingDescriptions),
        m_vertexAttributeDescriptions(other.m_vertexAttributeDescriptions) {
    setupBindingDescriptions();
    setupAttributeDescriptions();
  }

  PipelineVertexInputStateCreate(
      PipelineVertexInputStateCreate &&other) noexcept
      : VkPipelineVertexInputStateCreateInfo{other},
        m_vertexBindingDescriptions(
            std::move(other.m_vertexBindingDescriptions)),
        m_vertexAttributeDescriptions(
            std::move(other.m_vertexAttributeDescriptions)) {
    setupBindingDescriptions();
    setupAttributeDescriptions();
    other.setupBindingDescriptions();
    other.setupAttributeDescriptions();
  }
};
} // namespace info
} // namespace vk
