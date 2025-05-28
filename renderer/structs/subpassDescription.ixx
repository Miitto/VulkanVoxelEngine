module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:subpassDescription;

export namespace vk {
class SubpassDescription : public VkSubpassDescription {
  std::vector<VkAttachmentReference> colorReferences;
  std::vector<VkAttachmentReference> inputReferences;
  std::vector<VkAttachmentReference> resolveReferences;
  VkAttachmentReference depthReference;
  std::vector<uint32_t> preserveReferences;

  void setupInputAttachments() {
    inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
    pInputAttachments = inputReferences.data();
  }

  void setupColorAttachments() {
    colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
    pColorAttachments = colorReferences.data();
  }

  void setupResolveAttachments() {
    pResolveAttachments = resolveReferences.data();
  }

  void setupPreserveAttachments() {
    preserveAttachmentCount = static_cast<uint32_t>(preserveReferences.size());
    pPreserveAttachments = preserveReferences.data();
  }

public:
  SubpassDescription()
      : VkSubpassDescription{.flags = 0,
                             .pipelineBindPoint =
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                             .inputAttachmentCount = 0,
                             .pInputAttachments = nullptr,
                             .colorAttachmentCount = 0,
                             .pColorAttachments = nullptr,
                             .pResolveAttachments = nullptr,
                             .pDepthStencilAttachment = nullptr,
                             .preserveAttachmentCount = 0,
                             .pPreserveAttachments = nullptr} {}

  SubpassDescription &input(VkAttachmentReference attachment) {
    inputReferences.push_back(attachment);
    setupInputAttachments();
    return *this;
  }

  SubpassDescription &color(VkAttachmentReference attachment) {
    colorReferences.push_back(attachment);
    setupColorAttachments();
    return *this;
  }

  SubpassDescription &resolve(VkAttachmentReference attachment) {
    resolveReferences.push_back(attachment);
    setupResolveAttachments();
    return *this;
  }

  SubpassDescription &depthStencil(VkAttachmentReference &attachment) {
    depthReference = attachment;
    pDepthStencilAttachment = &depthReference;
    return *this;
  }

  SubpassDescription &preserve(uint32_t attachment) {
    preserveReferences.push_back(attachment);
    setupPreserveAttachments();
    return *this;
  }
};
} // namespace vk
