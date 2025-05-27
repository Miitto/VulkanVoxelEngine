module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:info.renderPassCreate;

export namespace vk {
namespace info {
class RenderPassCreate : public VkRenderPassCreateInfo {
  std::vector<VkAttachmentDescription> attachments;
  std::vector<VkSubpassDescription> subpasses;
  std::vector<VkSubpassDependency> dependencies;

  void setupAttachments() {
    attachmentCount = static_cast<uint32_t>(attachments.size());
    pAttachments = attachments.data();
  }

  void setupSubpasses() {
    subpassCount = static_cast<uint32_t>(subpasses.size());
    pSubpasses = subpasses.data();
  }

  void setupDependencies() {
    dependencyCount = static_cast<uint32_t>(dependencies.size());
    pDependencies = dependencies.data();
  }

public:
  RenderPassCreate()
      : VkRenderPassCreateInfo{.sType =
                                   VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                               .pNext = nullptr,
                               .flags = 0,
                               .attachmentCount = 0,
                               .pAttachments = nullptr,
                               .subpassCount = 0,
                               .pSubpasses = nullptr,
                               .dependencyCount = 0,
                               .pDependencies = nullptr} {}

  RenderPassCreate &addAttachment(VkAttachmentDescription attachment) {
    attachments.push_back(attachment);
    setupAttachments();
    return *this;
  }

  RenderPassCreate &addSubpass(VkSubpassDescription subpass) {
    subpasses.push_back(subpass);
    setupSubpasses();
    return *this;
  }

  RenderPassCreate &addDependency(VkSubpassDependency dependency) {
    dependencies.push_back(dependency);
    setupDependencies();
    return *this;
  }

  RenderPassCreate(const RenderPassCreate &other)
      : VkRenderPassCreateInfo{other}, attachments(other.attachments),
        subpasses(other.subpasses), dependencies(other.dependencies) {
    setupAttachments();
    setupSubpasses();
    setupDependencies();
  }

  RenderPassCreate(RenderPassCreate &&other) noexcept
      : VkRenderPassCreateInfo{other},
        attachments(std::move(other.attachments)),
        subpasses(std::move(other.subpasses)),
        dependencies(std::move(other.dependencies)) {
    setupAttachments();
    setupSubpasses();
    setupDependencies();
    other.setupAttachments();
    other.setupSubpasses();
    other.setupDependencies();
  }
};
} // namespace info
} // namespace vk
