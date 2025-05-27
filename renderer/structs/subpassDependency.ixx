module;

#include "vulkan/vulkan_core.h"

export module vk:subpassDependency;

export namespace vk {

class SubpassDependency : public VkSubpassDependency {

public:
  SubpassDependency(uint32_t srcSubpass = 0, uint32_t dstSubpass = 0,
                    VkPipelineStageFlags srcStageMask = 0,
                    VkPipelineStageFlags dstStageMask = 0,
                    VkAccessFlags srcAccessMask = 0,
                    VkAccessFlags dstAccessMask = 0,
                    VkDependencyFlags dependencyFlags = 0)
      : VkSubpassDependency{.srcSubpass = srcSubpass,
                            .dstSubpass = dstSubpass,
                            .srcStageMask = srcStageMask,
                            .dstStageMask = dstStageMask,
                            .srcAccessMask = srcAccessMask,
                            .dstAccessMask = dstAccessMask,
                            .dependencyFlags = dependencyFlags} {}

  SubpassDependency(VkSubpassDependency &&dependency)
      : VkSubpassDependency{dependency} {}
};
} // namespace vk
