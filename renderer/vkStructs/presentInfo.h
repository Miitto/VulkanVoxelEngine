#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class PresentInfoBuilder {
  VkPresentInfoKHR createInfo;
  std::vector<VkSwapchainKHR> swapchains;
  std::vector<VkSemaphore> waitSemaphores;

public:
  PresentInfoBuilder() {
    createInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.waitSemaphoreCount = 0;
    createInfo.pWaitSemaphores = nullptr;
    createInfo.swapchainCount = 0;
    createInfo.pSwapchains = nullptr;
    createInfo.pImageIndices = nullptr;
    createInfo.pResults = nullptr;
  }

  PresentInfoBuilder &addSwapchain(VkSwapchainKHR swapchain) {
    swapchains.push_back(swapchain);
    return *this;
  }

  PresentInfoBuilder &setImageIndex(uint32_t &index) {
    createInfo.pImageIndices = &index;
    return *this;
  }

  PresentInfoBuilder &setImageIndices(const std::vector<uint32_t> &indices) {
    createInfo.pImageIndices = indices.data();
    return *this;
  }

  PresentInfoBuilder &addWaitSemaphore(VkSemaphore semaphore) {
    waitSemaphores.push_back(semaphore);
    return *this;
  }

  VkPresentInfoKHR build() {
    createInfo.swapchainCount = static_cast<uint32_t>(swapchains.size());
    createInfo.pSwapchains = swapchains.data();
    createInfo.waitSemaphoreCount =
        static_cast<uint32_t>(waitSemaphores.size());
    createInfo.pWaitSemaphores = waitSemaphores.data();
    return createInfo;
  }
};
