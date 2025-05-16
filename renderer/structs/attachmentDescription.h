#pragma once

#include "vulkan/vulkan_core.h"

namespace vk {

class AttachmentDescription : public VkAttachmentDescription {
public:
  AttachmentDescription(VkFormat &format)
      : VkAttachmentDescription{
            .flags = 0,
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_UNDEFINED} {}

  AttachmentDescription &setColorDepth(VkAttachmentLoadOp colorDepthLoadOp,
                                       VkAttachmentStoreOp colorDepthStoreOp) {
    loadOp = colorDepthLoadOp;
    storeOp = colorDepthStoreOp;
    return *this;
  }

  AttachmentDescription &setStencil(VkAttachmentLoadOp loadOp,
                                    VkAttachmentStoreOp storeOp) {
    stencilLoadOp = loadOp;
    stencilStoreOp = storeOp;
    return *this;
  }

  AttachmentDescription &setInitialLayout(VkImageLayout layout) {
    initialLayout = layout;
    return *this;
  }

  AttachmentDescription &setFinalLayout(VkImageLayout layout) {
    finalLayout = layout;
    return *this;
  }
};
} // namespace vk
