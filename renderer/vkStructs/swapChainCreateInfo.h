#pragma once

#include "surface.h"
#include <iostream>
#include <vulkan/vulkan.h>

class SwapChainCreateInfo : public VkSwapchainCreateInfoKHR {
  SurfaceAttributes &swapChainSupport;

public:
  SwapChainCreateInfo(SurfaceAttributes &swapChainSupport, Surface &surface)
      : swapChainSupport(swapChainSupport) {
    sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    pNext = nullptr;
    this->surface = *surface;
    flags = 0;
    minImageCount = swapChainSupport.capabilities.minImageCount;
    imageFormat = swapChainSupport.formats[0].format;
    imageColorSpace = swapChainSupport.formats[0].colorSpace;
    imageExtent = swapChainSupport.capabilities.currentExtent;
    imageArrayLayers = 1;
    imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    preTransform = swapChainSupport.capabilities.currentTransform;
    compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    presentMode = swapChainSupport.presentModes[0];
    clipped = VK_TRUE;
    oldSwapchain = VK_NULL_HANDLE;
  }

  SwapChainCreateInfo &setImageCount(uint32_t count) {
    if (count < swapChainSupport.capabilities.minImageCount ||
        count > swapChainSupport.capabilities.maxImageCount) {
      std::cerr
          << "Attempted to set image count to below min or above max, aborting"
          << std::endl;
    }
    minImageCount = count;
    return *this;
  }

  SwapChainCreateInfo &setImageFormat(VkFormat format) {
    if (format == VK_FORMAT_UNDEFINED) {
      std::cerr << "Attempted to set image format to undefined, aborting"
                << std::endl;
    }

    for (const auto &availableFormat : swapChainSupport.formats) {
      if (availableFormat.format == format) {
        imageFormat = format;
        return *this;
      }
    }

    std::cerr << "Attempted to set image format to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfo &setImageColorSpace(VkColorSpaceKHR colorSpace) {
    if (colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR) {
      std::cerr << "Attempted to set image color space to undefined, aborting"
                << std::endl;
    }

    for (const auto &availableFormat : swapChainSupport.formats) {
      if (availableFormat.colorSpace == colorSpace) {
        imageColorSpace = colorSpace;
        return *this;
      }
    }

    std::cerr << "Attempted to set image color space to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfo &setImageExtent(VkExtent2D extent) {
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

    imageExtent = extent;
    return *this;
  }

  SwapChainCreateInfo &setImageArrayLayers(uint32_t layers) {
    if (layers < 1) {
      std::cerr << "Attempted to set image array layers to less than 1, "
                   "aborting"
                << std::endl;
    }

    imageArrayLayers = layers;
    return *this;
  }

  SwapChainCreateInfo &setPresentMode(VkPresentModeKHR presentMode) {
    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
      std::cerr << "Attempted to set present mode to undefined, aborting"
                << std::endl;
    }

    for (const auto &availablePresentMode : swapChainSupport.presentModes) {
      if (availablePresentMode == presentMode) {
        this->presentMode = presentMode;
        return *this;
      }
    }

    std::cerr << "Attempted to set present mode to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapChainCreateInfo &setClipped(VkBool32 clip) {
    clipped = clip;
    return *this;
  }

  SwapChainCreateInfo &setOldSwapchain(VkSwapchainKHR swapchain) {
    oldSwapchain = swapchain;
    return *this;
  }

  SwapChainCreateInfo &setImageSharingMode(VkSharingMode sharingMode) {
    imageSharingMode = sharingMode;
    return *this;
  }

  SwapChainCreateInfo &
  setQueueFamilyIndices(const std::vector<uint32_t> &indices) {
    if (indices.empty()) {
      std::cerr << "Attempted to set queue family indices to empty, aborting"
                << std::endl;
    }

    queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
    pQueueFamilyIndices = indices.data();
    return *this;
  }
};
