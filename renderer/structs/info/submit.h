#pragma once

#include "vulkan/vulkan_core.h"
#include <vector>

namespace vk {
namespace info {

class Submit : public VkSubmitInfo {
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkSemaphore> waitSemaphores;
  std::vector<VkSemaphore> signalSemaphores;
  std::vector<VkPipelineStageFlags> waitDstStageMasks;

  void setupCommandBuffers() {
    commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    pCommandBuffers = commandBuffers.empty() ? nullptr : commandBuffers.data();
  }

  void setupWaitSemaphores() {
    waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    pWaitSemaphores = waitSemaphores.empty() ? nullptr : waitSemaphores.data();
    pWaitDstStageMask =
        waitDstStageMasks.empty() ? nullptr : waitDstStageMasks.data();
  }

  void setupSignalSemaphores() {
    signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    pSignalSemaphores =
        signalSemaphores.empty() ? nullptr : signalSemaphores.data();
  }

public:
  Submit()
      : VkSubmitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                     .pNext = nullptr,
                     .waitSemaphoreCount = 0,
                     .pWaitSemaphores = nullptr,
                     .pWaitDstStageMask = nullptr,
                     .commandBufferCount = 0,
                     .pCommandBuffers = nullptr,
                     .signalSemaphoreCount = 0,
                     .pSignalSemaphores = nullptr} {}

  Submit &addCommandBuffer(VkCommandBuffer commandBuffer) {
    commandBuffers.push_back(commandBuffer);
    setupCommandBuffers();
    return *this;
  }

  Submit &addWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags stage) {
    waitSemaphores.push_back(semaphore);
    waitDstStageMasks.push_back(stage);
    setupWaitSemaphores();
    return *this;
  }

  Submit &addSignalSemaphore(VkSemaphore semaphore) {
    signalSemaphores.push_back(semaphore);
    setupSignalSemaphores();
    return *this;
  }

  Submit(const Submit &other)
      : VkSubmitInfo{other}, commandBuffers(other.commandBuffers),
        waitSemaphores(other.waitSemaphores),
        signalSemaphores(other.signalSemaphores),
        waitDstStageMasks(other.waitDstStageMasks) {
    setupCommandBuffers();
    setupWaitSemaphores();
    setupSignalSemaphores();
  }

  Submit(Submit &&other) noexcept
      : VkSubmitInfo{other}, commandBuffers(std::move(other.commandBuffers)),
        waitSemaphores(std::move(other.waitSemaphores)),
        signalSemaphores(std::move(other.signalSemaphores)),
        waitDstStageMasks(std::move(other.waitDstStageMasks)) {
    setupCommandBuffers();
    setupWaitSemaphores();
    setupSignalSemaphores();
    other.setupCommandBuffers();
    other.setupWaitSemaphores();
    other.setupSignalSemaphores();
  }
};
} // namespace info
} // namespace vk
