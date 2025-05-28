module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:info.framebufferCreate;

export namespace vk {
namespace info {
class FramebufferCreate : public VkFramebufferCreateInfo {
  std::vector<VkImageView> m_attachments;

  void setupAttachments() {
    attachmentCount = static_cast<uint32_t>(m_attachments.size());
    pAttachments = m_attachments.empty() ? nullptr : m_attachments.data();
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
    m_attachments.push_back(attachment);
    setupAttachments();
    return *this;
  }

  FramebufferCreate &addAttachments(std::vector<VkImageView> attchmnts) {
    m_attachments.insert(m_attachments.end(), attchmnts.begin(),
                             attchmnts.end());
    setupAttachments();
    return *this;
  }

  FramebufferCreate &setAttachments(std::vector<VkImageView> attachments) {
    this->m_attachments = std::move(attachments);
    setupAttachments();
    return *this;
  }

  FramebufferCreate(const FramebufferCreate &other)
      : VkFramebufferCreateInfo{other}, m_attachments(other.m_attachments) {
    setupAttachments();
  }

  FramebufferCreate(FramebufferCreate &&other) noexcept
      : VkFramebufferCreateInfo{other},
        m_attachments(std::move(other.m_attachments)) {
    setupAttachments();
    other.setupAttachments();
  }
};
} // namespace info
} // namespace vk
