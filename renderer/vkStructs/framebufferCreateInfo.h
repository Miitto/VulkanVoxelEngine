#pragma once

#include "vulkan/vulkan.h"
#include <print>
#include <vector>

class FramebufferCreateInfoBuilder {
  VkFramebufferCreateInfo createInfo;
  std::vector<VkImageView> attachments;

public:
  FramebufferCreateInfoBuilder(VkRenderPass renderPass, VkExtent2D extent)
      : FramebufferCreateInfoBuilder(renderPass, extent.width, extent.height) {}
  FramebufferCreateInfoBuilder(VkRenderPass renderPass, uint32_t width,
                               uint32_t height) {
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = 0;
    createInfo.pAttachments = nullptr;
    createInfo.width = width;
    createInfo.height = height;
    createInfo.layers = 1;
    createInfo.renderPass = renderPass;
  }

  FramebufferCreateInfoBuilder &addAttachment(VkImageView attachment) {
    attachments.push_back(attachment);
    return *this;
  }

  VkFramebufferCreateInfo build() {
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    return createInfo;
  }
};
