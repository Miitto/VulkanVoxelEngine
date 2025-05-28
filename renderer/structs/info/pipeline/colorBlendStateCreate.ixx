module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:info.pipeline.colorBlendStateCreate;

export namespace vk {
namespace info {
class PipelineColorBlendStateCreate
    : public VkPipelineColorBlendStateCreateInfo {
  std::vector<VkPipelineColorBlendAttachmentState> attachments;

  void setupAttachments() {
    attachmentCount = static_cast<uint32_t>(attachments.size());
    pAttachments = attachments.data();
  }

public:
  PipelineColorBlendStateCreate()
      : VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 0,
            .pAttachments = nullptr,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}} {}
  PipelineColorBlendStateCreate &
  addAttachment(VkPipelineColorBlendAttachmentState attachment) {
    attachments.push_back(attachment);
    setupAttachments();
    return *this;
  }

  PipelineColorBlendStateCreate(const PipelineColorBlendStateCreate &o) noexcept
      : VkPipelineColorBlendStateCreateInfo{o}, attachments{o.attachments} {
    setupAttachments();
  }

  PipelineColorBlendStateCreate(PipelineColorBlendStateCreate &&o) noexcept
      : VkPipelineColorBlendStateCreateInfo{o},
        attachments(std::move(o.attachments)) {
    o.setupAttachments();
    setupAttachments();
  }
};
} // namespace info
} // namespace vk
