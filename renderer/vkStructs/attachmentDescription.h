#pragma once

#include "vulkan/vulkan.h"

class AttachmentDescriptionBuilder {
  VkAttachmentDescription desc;

public:
  AttachmentDescriptionBuilder(VkFormat &format) {
    desc.flags = 0;
    desc.format = format;
    desc.samples = VK_SAMPLE_COUNT_1_BIT;
    desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }

  AttachmentDescriptionBuilder &setColorDepth(VkAttachmentLoadOp loadOp,
                                              VkAttachmentStoreOp storeOp) {
    desc.loadOp = loadOp;
    desc.storeOp = storeOp;
    return *this;
  }

  AttachmentDescriptionBuilder &setStencil(VkAttachmentLoadOp loadOp,
                                           VkAttachmentStoreOp storeOp) {
    desc.stencilLoadOp = loadOp;
    desc.stencilStoreOp = storeOp;
    return *this;
  }

  AttachmentDescriptionBuilder &setInitialLayout(VkImageLayout layout) {
    desc.initialLayout = layout;
    return *this;
  }

  AttachmentDescriptionBuilder &setFinalLayout(VkImageLayout layout) {
    desc.finalLayout = layout;
    return *this;
  }

  VkAttachmentDescription build() { return desc; }
};
