#pragma once
#include "logicalDevice.h"
#include "vkStructs/imageViewCreate.h"
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

class SwapChain {
  Device::Ref device;
  VkSwapchainKHR swapChain;
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  VkExtent2D extent;
  VkFormat format;

public:
  SwapChain(VkSwapchainKHR swapChain, Device &device,
            std::vector<VkImage> &images, std::vector<VkImageView> &imageViews,
            VkExtent2D extent, VkFormat format)
      : swapChain(swapChain), device(device.ref()), images(images),
        imageViews(imageViews), extent(extent), format(format) {}

  ~SwapChain() {
    if (swapChain != VK_NULL_HANDLE) {
      for (auto imageView : imageViews) {
        vkDestroyImageView(**device, imageView, nullptr);
      }
      vkDestroySwapchainKHR(**device, swapChain, nullptr);
    }
  }

  SwapChain(const SwapChain &) = delete;
  SwapChain &operator=(const SwapChain &) = delete;

  SwapChain(SwapChain &&o) noexcept
      : swapChain(o.swapChain), device(o.device), images(std::move(o.images)),
        imageViews(std::move(o.imageViews)), extent(o.extent),
        format(o.format) {
    o.swapChain = VK_NULL_HANDLE;
  }
  SwapChain &operator=(SwapChain &&o) = delete;

  VkSwapchainKHR &operator*() { return swapChain; }
  std::vector<VkImage> &getImages() { return images; }
  std::vector<VkImageView> &getImageViews() { return imageViews; }

public:
  static std::optional<SwapChain> create(VkSwapchainKHR swapChain,
                                         Device &device, VkFormat format,
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
    return SwapChain(swapChain, device, images, imageViews, extent, format);
  }

  VkExtent2D &getExtent() { return extent; }
  VkFormat &getFormat() { return format; }
};
