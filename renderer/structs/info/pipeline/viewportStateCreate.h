#pragma once

#include "vulkan/vulkan_core.h"
#include <vector>

namespace vk {
namespace info {
class PiplineViewportStateCreate : public VkPipelineViewportStateCreateInfo {
  std::vector<VkViewport> viewports;
  std::vector<VkRect2D> scissors;

  void setupViewports() {
    viewportCount = static_cast<uint32_t>(viewports.size());
    pViewports = viewports.data();
  }

  void setupScissors() {
    scissorCount = static_cast<uint32_t>(scissors.size());
    pScissors = scissors.data();
  }

public:
  PiplineViewportStateCreate()
      : VkPipelineViewportStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 0,
            .pViewports = nullptr,
            .scissorCount = 0,
            .pScissors = nullptr} {}

  PiplineViewportStateCreate &addViewport(const VkViewport &viewport) {
    viewports.push_back(viewport);

    setupViewports();

    return *this;
  }

  PiplineViewportStateCreate &addScissor(const VkRect2D &scissor) {
    scissors.push_back(scissor);

    setupScissors();

    return *this;
  }

  PiplineViewportStateCreate(const PiplineViewportStateCreate &other)
      : VkPipelineViewportStateCreateInfo{other}, viewports(other.viewports),
        scissors(other.scissors) {
    setupViewports();
    setupScissors();
  }

  PiplineViewportStateCreate(PiplineViewportStateCreate &&other) noexcept
      : VkPipelineViewportStateCreateInfo{other},
        viewports(std::move(other.viewports)),
        scissors(std::move(other.scissors)) {
    setupViewports();
    setupScissors();
    other.setupViewports();
    other.setupScissors();
  }
};
} // namespace info
} // namespace vk
