module;

#include "vulkan/vulkan_core.h"
#include <vector>

export module vk:info.commands.renderPassBegin;

export namespace vk {
namespace info {
class RenderPassBegin : public VkRenderPassBeginInfo {
  std::vector<VkClearValue> clearValues;

  void setupClearValues() {
    clearValueCount = static_cast<uint32_t>(clearValues.size());
    pClearValues = clearValues.data();
  }

public:
  RenderPassBegin(VkRenderPass renderPass, VkFramebuffer framebuffer,
                  VkRect2D renderArea)
      : VkRenderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .renderArea = renderArea,
            .clearValueCount = 0,
            .pClearValues = nullptr,
        } {}

  RenderPassBegin &addClearValue(const VkClearValue &clearValue) {
    clearValues.push_back(clearValue);
    setupClearValues();
    return *this;
  }

  RenderPassBegin(const RenderPassBegin &o) noexcept
      : VkRenderPassBeginInfo{o}, clearValues(o.clearValues) {
    setupClearValues();
  }

  RenderPassBegin(RenderPassBegin &&o) noexcept
      : VkRenderPassBeginInfo{o}, clearValues(std::move(o.clearValues)) {
    o.setupClearValues();
    setupClearValues();
  }
};
} // namespace info
} // namespace vk
