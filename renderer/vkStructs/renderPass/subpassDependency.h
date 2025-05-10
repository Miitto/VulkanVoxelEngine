#pragma once

#include "vulkan/vulkan.h"

class SubpassDependencyBuilder {
  VkSubpassDependency dependency;

public:
  SubpassDependencyBuilder(uint32_t srcSubpass, uint32_t dstSubpass) {
    dependency.srcSubpass = srcSubpass;
    dependency.dstSubpass = dstSubpass;
    dependency.srcStageMask = 0;
    dependency.dstStageMask = 0;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = 0;
    dependency.dependencyFlags = 0;
  }

  SubpassDependencyBuilder &setSrcStageMask(VkPipelineStageFlags mask) {
    dependency.srcStageMask = mask;
    return *this;
  }

  SubpassDependencyBuilder &setDstStageMask(VkPipelineStageFlags mask) {
    dependency.dstStageMask = mask;
    return *this;
  }

  SubpassDependencyBuilder &setSrcAccessMask(VkAccessFlags mask) {
    dependency.srcAccessMask = mask;
    return *this;
  }

  SubpassDependencyBuilder &setDstAccessMask(VkAccessFlags mask) {
    dependency.dstAccessMask = mask;
    return *this;
  }

  SubpassDependencyBuilder &setDependencyFlags(VkDependencyFlags flags) {
    dependency.dependencyFlags = flags;
    return *this;
  }

  VkSubpassDependency build() { return dependency; }
};
