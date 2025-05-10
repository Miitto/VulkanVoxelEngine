#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class SubmitInfoBuilder {
  VkSubmitInfo createInfo;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkSemaphore> waitSemaphores;
  std::vector<VkSemaphore> signalSemaphores;
  std::vector<VkPipelineStageFlags> waitDstStageMasks;

public:
  SubmitInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    createInfo.pNext = nullptr;
    createInfo.waitSemaphoreCount = 0;
    createInfo.pWaitSemaphores = nullptr;
    createInfo.pWaitDstStageMask = nullptr;
    createInfo.commandBufferCount = 0;
    createInfo.pCommandBuffers = nullptr;
    createInfo.signalSemaphoreCount = 0;
    createInfo.pSignalSemaphores = nullptr;
  }

  SubmitInfoBuilder &addCommandBuffer(VkCommandBuffer commandBuffer) {
    commandBuffers.push_back(commandBuffer);
    return *this;
  }

  SubmitInfoBuilder &addWaitSemaphore(VkSemaphore semaphore,
                                      VkPipelineStageFlags stage) {
    waitSemaphores.push_back(semaphore);
    waitDstStageMasks.push_back(stage);
    return *this;
  }

  SubmitInfoBuilder &addSignalSemaphore(VkSemaphore semaphore) {
    signalSemaphores.push_back(semaphore);
    return *this;
  }

  VkSubmitInfo build() {
    createInfo.commandBufferCount =
        static_cast<uint32_t>(commandBuffers.size());
    createInfo.pCommandBuffers =
        commandBuffers.empty() ? nullptr : commandBuffers.data();
    createInfo.waitSemaphoreCount =
        static_cast<uint32_t>(waitSemaphores.size());
    createInfo.pWaitSemaphores =
        waitSemaphores.empty() ? nullptr : waitSemaphores.data();
    createInfo.pWaitDstStageMask = waitDstStageMasks.data();
    createInfo.signalSemaphoreCount =
        static_cast<uint32_t>(signalSemaphores.size());
    createInfo.pSignalSemaphores =
        signalSemaphores.empty() ? nullptr : signalSemaphores.data();

    return createInfo;
  }
};
