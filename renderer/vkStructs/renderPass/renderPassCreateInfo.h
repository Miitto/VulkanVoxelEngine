#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class RenderPassCreateInfoBuilder {
  VkRenderPassCreateInfo createInfo;
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkSubpassDescription> subpasses;
  std::vector<VkSubpassDependency> dependencies;

public:
  RenderPassCreateInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments = nullptr;
    createInfo.subpassCount = 0;
    createInfo.pSubpasses = nullptr;
  }

  RenderPassCreateInfoBuilder &
  addAttachment(VkAttachmentDescription attachment) {
    attachments.push_back(attachment);
    return *this;
  }

  RenderPassCreateInfoBuilder &addSubpass(VkSubpassDescription subpass) {
    subpasses.push_back(subpass);
    return *this;
  }

  RenderPassCreateInfoBuilder &addDependency(VkSubpassDependency dependency) {
    dependencies.push_back(dependency);
    return *this;
  }

  VkRenderPassCreateInfo build() {
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    createInfo.pSubpasses = subpasses.data();
    createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    createInfo.pDependencies = dependencies.data();
    return createInfo;
  }
};
