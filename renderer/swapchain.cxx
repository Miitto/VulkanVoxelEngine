module;

#include <optional>

#include "log.h"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

module vk;

import :info.framebufferCreate;
import :info.imageViewCreate;

namespace vk {
std::optional<Swapchain> Swapchain::create(Device &device,
                                           vk::info::SwapchainCreate info) {
  VkSwapchainKHR swapChain;
  if (vkCreateSwapchainKHR(*device, &info, nullptr, &swapChain) != VK_SUCCESS) {
    LOG_ERR("Failed to create swap chain");
    return std::nullopt;
  }

  uint32_t imageCount;
  vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, nullptr);
  std::vector<VkImage> images(imageCount);
  vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, images.data());

  std::vector<VkImageView> imageViews(imageCount);
  for (size_t i = 0; i < imageCount; ++i) {
    auto createInfo = vk::info::ImageViewCreate(images[i], info.imageFormat);
    if (vkCreateImageView(*device, &createInfo, nullptr, &imageViews[i]) !=
        VK_SUCCESS) {
      LOG_ERR("Failed to create image views!");
      return std::nullopt;
    }
  }
  return Swapchain(swapChain, device, images, imageViews, info.imageExtent,
                   info.imageFormat);
}

std::optional<std::vector<Framebuffer>>
Swapchain::createFramebuffers(RenderPass &renderPass) {
  std::vector<Framebuffer> framebuffers;
  for (auto &imageView : imageViews) {
    vk::info::FramebufferCreate builder(*renderPass, extent.width,
                                        extent.height);
    auto framebufferCreateInfo = builder.addAttachment(imageView);

    auto framebuffer = Framebuffer::create(*device, framebufferCreateInfo);
    if (!framebuffer.has_value()) {
      LOG_ERR("Failed to create framebuffer!");
      return std::nullopt;
    }
    framebuffers.push_back(std::move(framebuffer.value()));
  }

  return framebuffers;
}
} // namespace vk
