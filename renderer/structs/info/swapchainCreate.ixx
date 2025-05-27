module;

#include <iostream>
#include <vulkan/vulkan.h>

export module vk:structs.info.swapchainCreate;

import :surface;

namespace vk {
namespace info {

class SwapchainCreate : public VkSwapchainCreateInfoKHR {
  SurfaceAttributes &swapChainSupport;

public:
  SwapchainCreate(SurfaceAttributes &swapChainSupport, Surface &surface,
                  bool share = false)
      : VkSwapchainCreateInfoKHR{.sType =
                                     VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .surface = *surface,
                                 .minImageCount = swapChainSupport.capabilities
                                                      .minImageCount,
                                 .imageFormat =
                                     swapChainSupport.formats[0].format,
                                 .imageColorSpace =
                                     swapChainSupport.formats[0].colorSpace,
                                 .imageExtent = swapChainSupport.capabilities
                                                    .currentExtent,
                                 .imageArrayLayers = 1,
                                 .imageUsage =
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 .imageSharingMode =
                                     share ? VK_SHARING_MODE_CONCURRENT
                                           : VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 0,
                                 .pQueueFamilyIndices = nullptr,
                                 .preTransform = swapChainSupport.capabilities
                                                     .currentTransform,
                                 .compositeAlpha =
                                     VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                 .presentMode =
                                     swapChainSupport.presentModes[0],
                                 .clipped = VK_TRUE,
                                 .oldSwapchain = VK_NULL_HANDLE},
        swapChainSupport(swapChainSupport) {}

  SwapchainCreate &setImageCount(uint32_t count) {
    if (count < swapChainSupport.capabilities.minImageCount ||
        count > swapChainSupport.capabilities.maxImageCount) {
      std::cerr << "Attempted to set image count to below "
                   "min or above max, aborting"
                << std::endl;
    }
    minImageCount = count;
    return *this;
  }

  SwapchainCreate &setImageFormat(VkFormat format) {
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

  SwapchainCreate &setImageColorSpace(VkColorSpaceKHR colorSpace) {
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

  SwapchainCreate &setImageExtent(VkExtent2D extent) {
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

  SwapchainCreate &setImageArrayLayers(uint32_t layers) {
    if (layers < 1) {
      std::cerr << "Attempted to set image array layers to less than 1, "
                   "aborting"
                << std::endl;
    }

    imageArrayLayers = layers;
    return *this;
  }

  SwapchainCreate &setPresentMode(VkPresentModeKHR present_mode) {
    for (const auto &availablePresentMode : swapChainSupport.presentModes) {
      if (availablePresentMode == present_mode) {
        presentMode = present_mode;
        return *this;
      }
    }

    std::cerr << "Attempted to set present mode to unsupported format, "
                 "aborting"
              << std::endl;

    return *this;
  }

  SwapchainCreate &setClipped(VkBool32 clip) {
    clipped = clip;
    return *this;
  }

  SwapchainCreate &setOldSwapchain(VkSwapchainKHR swapchain) {
    oldSwapchain = swapchain;
    return *this;
  }

  SwapchainCreate &setImageSharingMode(VkSharingMode sharingMode) {
    imageSharingMode = sharingMode;
    return *this;
  }

  SwapchainCreate &setQueueFamilyIndices(const std::vector<uint32_t> &indices) {
    if (indices.empty()) {
      std::cerr << "Attempted to set queue family indices to empty, aborting"
                << std::endl;
    }

    queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
    pQueueFamilyIndices = indices.data();
    return *this;
  }
};
} // namespace info
} // namespace vk
