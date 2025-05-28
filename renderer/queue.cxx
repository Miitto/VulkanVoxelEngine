module;

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
module vk;
namespace vk {
VkResult Queue::submit(CommandBuffer &cmdBuffer, VkFence fence) {
  vk::info::Submit submitInfo;
  submitInfo.addCommandBuffer(*cmdBuffer);
  return submit(submitInfo, fence);
}
} // namespace vk
