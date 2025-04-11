#pragma once
#include "../logicalDevice.h"
#include <iostream>
#include <vulkan/vulkan.h>

#include "../swapChain.h"

class SwapChainCreateInfoBuilder {
  VkSwapchainCreateInfoKHR createInfo;
  SwapChainSupport &swapChainSupport;
  Surface surface;

public:
  SwapChainCreateInfoBuilder(SwapChainSupport &swapChainSupport,
                             Surface &surface)
      : surface(surface), swapChainSupport(swapChainSupport) {
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.surface = *surface;
    createInfo.flags = 0;
    createInfo.minImageCount = swapChainSupport.capabilities.minImageCount;
    createInfo.imageFormat = swapChainSupport.formats[0].format;
    createInfo.imageColorSpace = swapChainSupport.formats[0].colorSpace;
    createInfo.imageExtent = swapChainSupport.capabilities.currentExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = swapChainSupport.presentModes[0];
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
  }

  SwapChainCreateInfoBuilder &setImageCount(uint32_t count) {
    if (count < swapChainSupport.capabilities.minImageCount ||
        count > swapChainSupport.capabilities.maxImageCount) {
      std::cerr
          << "Attempted to set image count to below min or above max, aborting"
          << std::endl;
    }
    createInfo.minImageCount = count;
    return *this;
  }

  SwapChainCreateInfoBuilder &setImageFormat(VkFormat format) {
    if (format == VK_FORMAT_UNDEFINED) {
      std::cerr << "Attempted to set image format to undefined, aborting"
                << std::endl;
    }

    for (const auto &availableFormat : swapChainSupport.formats) {
      if (availableFormat.format == format) {
        createInfo.imageFormat = format;
        return *this;
      }
    }

    std::cerr << "Attempted to set image format to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfoBuilder &setImageColorSpace(VkColorSpaceKHR colorSpace) {
    if (colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR) {
      std::cerr << "Attempted to set image color space to undefined, aborting"
                << std::endl;
    }

    for (const auto &availableFormat : swapChainSupport.formats) {
      if (availableFormat.colorSpace == colorSpace) {
        createInfo.imageColorSpace = colorSpace;
        return *this;
      }
    }

    std::cerr << "Attempted to set image color space to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfoBuilder &setImageExtent(VkExtent2D extent) {
    if (extent.width == 0 || extent.height == 0) {
      std::cerr << "Attempted to set image extent to undefined, aborting"
                << std::endl;
    }

    if (extent.width < swapChainSupport.capabilities.minImageExtent.width ||
        extent.width > swapChainSupport.capabilities.maxImageExtent.width ||
        extent.height < swapChainSupport.capabilities.minImageExtent.height ||
        extent.height > swapChainSupport.capabilities.maxImageExtent.height) {
      std::cerr << "Attempted to set image extent to unsupported format, "
                   "aborting"
                << std::endl;
    }

    createInfo.imageExtent = extent;
    return *this;
  }

  SwapChainCreateInfoBuilder &setImageArrayLayers(uint32_t layers) {
    if (layers < 1) {
      std::cerr << "Attempted to set image array layers to less than 1, "
                   "aborting"
                << std::endl;
    }

    createInfo.imageArrayLayers = layers;
    return *this;
  }

  SwapChainCreateInfoBuilder &setPresentMode(VkPresentModeKHR presentMode) {
    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
      std::cerr << "Attempted to set present mode to undefined, aborting"
                << std::endl;
    }

    for (const auto &availablePresentMode : swapChainSupport.presentModes) {
      if (availablePresentMode == presentMode) {
        createInfo.presentMode = presentMode;
        return *this;
      }
    }

    std::cerr << "Attempted to set present mode to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfoBuilder &setClipped(VkBool32 clipped) {
    createInfo.clipped = clipped;
    return *this;
  }

  SwapChainCreateInfoBuilder &setOldSwapChain(SwapChain &oldSwapChain) {
    createInfo.oldSwapchain = &**oldSwapChain;
    return *this;
  }

  SwapChainCreateInfoBuilder &setImageSharingMode(VkSharingMode sharingMode) {
    createInfo.imageSharingMode = sharingMode;
    return *this;
  }

  SwapChainCreateInfoBuilder &
  setQueueFamilyIndices(const std::vector<uint32_t> &indices) {
    if (indices.empty()) {
      std::cerr << "Attempted to set queue family indices to empty, aborting"
                << std::endl;
    }

    createInfo.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
    createInfo.pQueueFamilyIndices = indices.data();
    return *this;
  }

  std::optional<SwapChain> build(LogicalDevice &logicalDevice) {
    VkSwapchainKHR swapChain;
    if (vkCreateSwapchainKHR(*logicalDevice, &createInfo, nullptr,
                             &swapChain) != VK_SUCCESS) {
      std::cerr << "Failed to create swap chain." << std::endl;
      return std::nullopt;
    }
    return SwapChain(swapChain, logicalDevice);
  }
};
