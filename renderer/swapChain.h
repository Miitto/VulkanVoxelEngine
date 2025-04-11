#pragma once
#include "logicalDevice.h"
#include "physicalDevice.h"
#include "surface.h"
#include <vector>
#include <vulkan/vulkan.h>

class SwapChainSupport {
public:
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;

  SwapChainSupport(PhysicalDevice &device, Surface &surface);
};

class SwapChainPtr {
  LogicalDevice device;
  VkSwapchainKHR swapChain;

public:
  SwapChainPtr(VkSwapchainKHR swapChain, LogicalDevice device)
      : swapChain(swapChain), device(device) {}
  ~SwapChainPtr() {
    if (swapChain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(*device, swapChain, nullptr);
    }
  }

  SwapChainPtr(const SwapChainPtr &) = delete;
  SwapChainPtr &operator=(const SwapChainPtr &) = delete;

  SwapChainPtr(SwapChainPtr &&o) noexcept
      : swapChain(std::move(o.swapChain)), device(o.device) {
    o.swapChain = VK_NULL_HANDLE;
  }
  SwapChainPtr &operator=(SwapChainPtr &&o) noexcept {
    swapChain = std::move(o.swapChain);
    o.swapChain = VK_NULL_HANDLE;
    return *this;
  }

  VkSwapchainKHR &operator*() { return swapChain; }
};

class SwapChain {
  std::shared_ptr<SwapChainPtr> swapChain;

public:
  SwapChain(VkSwapchainKHR swapChain, LogicalDevice device)
      : swapChain(std::make_shared<SwapChainPtr>(swapChain, device)) {}

  VkSwapchainKHR &operator*() { return **swapChain; }
};
