#pragma once

#include "vulkan/vulkan_core.h"
#include <vector>

namespace vk {
namespace info {

class Present : public VkPresentInfoKHR {
  std::vector<VkSwapchainKHR> swapchains;
  std::vector<VkSemaphore> waitSemaphores;

  void setupSwapchains() {
    swapchainCount = static_cast<uint32_t>(swapchains.size());
    pSwapchains = swapchains.empty() ? nullptr : swapchains.data();
  }

  void setupWaitSemaphores() {
    waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    pWaitSemaphores = waitSemaphores.empty() ? nullptr : waitSemaphores.data();
  }

public:
  Present()
      : VkPresentInfoKHR{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                         .pNext = nullptr,
                         .waitSemaphoreCount = 0,
                         .pWaitSemaphores = nullptr,
                         .swapchainCount = 0,
                         .pSwapchains = nullptr,
                         .pImageIndices = nullptr,
                         .pResults = nullptr} {}

  Present &addSwapchain(VkSwapchainKHR swapchain) {
    swapchains.push_back(swapchain);

    setupSwapchains();
    return *this;
  }

  Present &setImageIndex(uint32_t &index) {
    pImageIndices = &index;
    return *this;
  }

  Present &setImageIndices(const std::vector<uint32_t> &indices) {
    pImageIndices = indices.data();
    return *this;
  }

  Present &addWaitSemaphore(VkSemaphore semaphore) {
    waitSemaphores.push_back(semaphore);
    setupWaitSemaphores();
    return *this;
  }

  Present(const Present &other)
      : VkPresentInfoKHR{other}, swapchains(other.swapchains),
        waitSemaphores(other.waitSemaphores) {
    setupSwapchains();
    setupWaitSemaphores();
  }

  Present(Present &&other) noexcept
      : VkPresentInfoKHR{other}, swapchains(std::move(other.swapchains)),
        waitSemaphores(std::move(other.waitSemaphores)) {
    setupSwapchains();
    setupWaitSemaphores();
    other.setupSwapchains();
    other.setupWaitSemaphores();
  }
};
} // namespace info
} // namespace vk
