#pragma once
#include "device/device.h"
#include "framebuffer.h"
#include "pipeline/renderPass.h"
#include "structs/info/swapchainCreate.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

class Swapchain {
  Device::Ref device;
  VkSwapchainKHR swapchain;
  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;
  VkExtent2D extent;
  VkFormat format;

public:
  Swapchain(VkSwapchainKHR swapchain, Device &device,
            std::vector<VkImage> &images, std::vector<VkImageView> &imageViews,
            VkExtent2D extent, VkFormat format)
      : device(device.ref()), swapchain(swapchain), images(images),
        imageViews(imageViews), extent(extent), format(format) {}

  ~Swapchain() {
    if (swapchain != VK_NULL_HANDLE) {
      for (auto imageView : imageViews) {
        vkDestroyImageView(**device, imageView, nullptr);
      }
      vkDestroySwapchainKHR(**device, swapchain, nullptr);
    }
  }

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  Swapchain(Swapchain &&o) noexcept
      : device(o.device), swapchain(o.swapchain), images(std::move(o.images)),
        imageViews(std::move(o.imageViews)), extent(o.extent),
        format(o.format) {
    o.swapchain = VK_NULL_HANDLE;
  }
  Swapchain &operator=(Swapchain &&o) = delete;

  VkSwapchainKHR &operator*() { return swapchain; }
  std::vector<VkImage> &getImages() { return images; }
  std::vector<VkImageView> &getImageViews() { return imageViews; }

public:
  static std::optional<Swapchain> create(Device &device,
                                         vk::info::SwapchainCreate info);
  VkExtent2D &getExtent() { return extent; }
  VkFormat &getFormat() { return format; }

  std::optional<std::vector<Framebuffer>>
  createFramebuffers(RenderPass &renderPass);

  struct SwapchainImageState {
    VkResult state;
    uint32_t imageIndex;
  };

  SwapchainImageState getNextImage(VkSemaphore semaphore = VK_NULL_HANDLE,
                                   VkFence fence = VK_NULL_HANDLE,
                                   uint64_t timeout = UINT64_MAX) {
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(**device, swapchain, timeout,
                                            semaphore, fence, &imageIndex);
    return {result, imageIndex};
  }
};
