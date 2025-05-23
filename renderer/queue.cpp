#include "queue.h"
#include "commands/buffer.h"

VkResult Queue::submit(CommandBuffer &cmdBuffer, VkFence fence) {
  vk::info::Submit submitInfo;
  submitInfo.addCommandBuffer(*cmdBuffer);
  return submit(submitInfo, fence);
}
