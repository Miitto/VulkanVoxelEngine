#pragma once
#include "logicalDevice.h"
#include "physicalDevice.h"
#include "surface.h"
#include "vkStructs/imageViewCreate.h"
#include <iostream>
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
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;

public:
  SwapChainPtr(VkSwapchainKHR swapChain, LogicalDevice device,
               std::vector<VkImage> &images,
               std::vector<VkImageView> &imageViews)
      : swapChain(swapChain), device(device), images(images),
        imageViews(imageViews) {}
  ~SwapChainPtr() {
    if (swapChain != VK_NULL_HANDLE) {
      for (auto imageView : imageViews) {
        vkDestroyImageView(*device, imageView, nullptr);
      }
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
  std::vector<VkImage> &getImages() { return images; }
  std::vector<VkImageView> &getImageViews() { return imageViews; }
};

class SwapChain {
  std::shared_ptr<SwapChainPtr> swapChain;
  VkFormat format;
  VkExtent2D extent;
  SwapChain(VkSwapchainKHR swapChain, LogicalDevice device,
            std::vector<VkImage> &images, std::vector<VkImageView> &imageViews,
            VkFormat format, VkExtent2D extent)
      : swapChain(std::make_shared<SwapChainPtr>(swapChain, device, images,
                                                 imageViews)),
        format(format), extent(extent) {}

public:
  static std::optional<SwapChain> create(VkSwapchainKHR swapChain,
                                         LogicalDevice device, VkFormat format,
                                         VkExtent2D extent) {
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, images.data());

    std::vector<VkImageView> imageViews(imageCount);
    for (int i = 0; i < imageCount; ++i) {
      auto createInfo = ImageViewCreateInfoBuilder(images[i], format).build();
      if (vkCreateImageView(*device, &createInfo, nullptr, &imageViews[i]) !=
          VK_SUCCESS) {
        std::cerr << "Failed to create image views!" << std::endl;
        return std::nullopt;
      }
    }

    return SwapChain(swapChain, device, images, imageViews, format, extent);
  }

  VkSwapchainKHR &operator*() { return **swapChain; }
  VkExtent2D &getExtent() { return extent; }
  VkFormat &getFormat() { return format; }
};
