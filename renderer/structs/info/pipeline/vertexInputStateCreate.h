#pragma once

#include "structs/vertexInputAttributeDescription.h"
#include "structs/vertexInputBindingDescription.h"
#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vk {
namespace info {
class PipelineVertexInputStateCreate
    : public VkPipelineVertexInputStateCreateInfo {

  std::vector<vk::VertexInputBindingDescription> m_vertexBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription>
      m_vertexAttributeDescriptions;

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
      const vk::VertexInputBindingDescription &bindingDescription) {
    m_vertexBindingDescriptions.push_back(bindingDescription);
    setupBindingDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &addBindingDescriptions(
      const std::span<vk::VertexInputBindingDescription> &bindingDescriptions) {
    m_vertexBindingDescriptions.insert(m_vertexBindingDescriptions.end(),
                                       bindingDescriptions.begin(),
                                       bindingDescriptions.end());
    setupBindingDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &addAttributeDescription(
      const vk::VertexInputAttributeDescription &attributeDescription) {
    m_vertexAttributeDescriptions.push_back(attributeDescription);
    setupAttributeDescriptions();
    return *this;
  }

  PipelineVertexInputStateCreate &
  addAttributeDescriptions(const std::span<vk::VertexInputAttributeDescription>
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
