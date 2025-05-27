module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:info.framebufferCreate;

export namespace vk {
namespace info {
class FramebufferCreate : public VkFramebufferCreateInfo {
  std::vector<VkImageView> attachments;

  void setupAttachments() {
    attachmentCount = static_cast<uint32_t>(attachments.size());
    pAttachments = attachments.empty() ? nullptr : attachments.data();
  }

public:
  FramebufferCreate(VkRenderPass renderPass, VkExtent2D extent)
      : FramebufferCreate(renderPass, extent.width, extent.height) {}
  FramebufferCreate(VkRenderPass renderPass, uint32_t width, uint32_t height)
      : VkFramebufferCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = 0,
                                .renderPass = renderPass,
                                .attachmentCount = 0,
                                .pAttachments = nullptr,
                                .width = width,
                                .height = height,
                                .layers = 1} {}

  FramebufferCreate &addAttachment(VkImageView attachment) {
    attachments.push_back(attachment);
    setupAttachments();
    return *this;
  }

  FramebufferCreate &addAttachments(std::vector<VkImageView> attachments) {
    this->attachments.insert(this->attachments.end(), attachments.begin(),
                             attachments.end());
    setupAttachments();
    return *this;
  }

  FramebufferCreate &setAttachments(std::vector<VkImageView> attachments) {
    this->attachments = std::move(attachments);
    setupAttachments();
    return *this;
  }

  FramebufferCreate(const FramebufferCreate &other)
      : VkFramebufferCreateInfo{other}, attachments(other.attachments) {
    setupAttachments();
  }

  FramebufferCreate(FramebufferCreate &&other) noexcept
      : VkFramebufferCreateInfo{other},
        attachments(std::move(other.attachments)) {
    setupAttachments();
    other.setupAttachments();
  }
};
} // namespace info
} // namespace vk
