#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class SubpassDescriptionBuilder {
  VkSubpassDescription desc;
  std::vector<VkAttachmentReference> colorReferences;
  std::vector<VkAttachmentReference> inputReferences;
  std::vector<VkAttachmentReference> resolveReferences;
  std::vector<uint32_t> preserveReferences;

public:
  SubpassDescriptionBuilder() {
    desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    desc.flags = 0;
    desc.inputAttachmentCount = 0;
    desc.pInputAttachments = nullptr;
    desc.colorAttachmentCount = 0;
    desc.pColorAttachments = nullptr;
    desc.pResolveAttachments = nullptr;
    desc.pDepthStencilAttachment = nullptr;
    desc.preserveAttachmentCount = 0;
    desc.pPreserveAttachments = nullptr;
  }

  SubpassDescriptionBuilder &
  setPipelineBindPoint(VkPipelineBindPoint bindPoint) {
    desc.pipelineBindPoint = bindPoint;
    return *this;
  }

  SubpassDescriptionBuilder &input(VkAttachmentReference attachment) {
    inputReferences.push_back(attachment);
    return *this;
  }

  SubpassDescriptionBuilder &color(VkAttachmentReference attachment) {
    colorReferences.push_back(attachment);
    return *this;
  }

  SubpassDescriptionBuilder &resolve(VkAttachmentReference attachment) {
    resolveReferences.push_back(attachment);
    return *this;
  }

  SubpassDescriptionBuilder &depthStencil(VkAttachmentReference attachment) {
    desc.pDepthStencilAttachment = &attachment;
    return *this;
  }

  SubpassDescriptionBuilder &preserve(uint32_t attachment) {
    preserveReferences.push_back(attachment);
    return *this;
  }

  VkSubpassDescription build() {
    desc.inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
    desc.pInputAttachments =
        inputReferences.size() > 0 ? inputReferences.data() : nullptr;
    desc.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
    desc.pColorAttachments =
        colorReferences.size() > 0 ? colorReferences.data() : nullptr;

    desc.pResolveAttachments =
        resolveReferences.size() > 0 ? resolveReferences.data() : nullptr;
    desc.preserveAttachmentCount = preserveReferences.size();
    desc.pPreserveAttachments =
        preserveReferences.size() > 0 ? preserveReferences.data() : nullptr;

    return desc;
  }
};
