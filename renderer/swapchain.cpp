#include "swapchain.h"
#include "vkStructs/framebufferCreateInfo.h"

std::optional<Swapchain> Swapchain::create(Device &device,
                                           VkSwapchainCreateInfoKHR info) {
  VkSwapchainKHR swapChain;
  if (vkCreateSwapchainKHR(*device, &info, nullptr, &swapChain) != VK_SUCCESS) {
    std::cerr << "Failed to create swap chain!" << std::endl;
    return std::nullopt;
  }

  uint32_t imageCount;
  vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, nullptr);
  std::vector<VkImage> images(imageCount);
  vkGetSwapchainImagesKHR(*device, swapChain, &imageCount, images.data());

  std::vector<VkImageView> imageViews(imageCount);
  for (size_t i = 0; i < imageCount; ++i) {
    auto createInfo =
        ImageViewCreateInfoBuilder(images[i], info.imageFormat).build();
    if (vkCreateImageView(*device, &createInfo, nullptr, &imageViews[i]) !=
        VK_SUCCESS) {
      std::cerr << "Failed to create image views!" << std::endl;
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
    FramebufferCreateInfoBuilder builder(*renderPass, extent.width,
                                         extent.height);
    auto framebufferCreateInfo = builder.addAttachment(imageView).build();

    auto framebuffer = Framebuffer::create(*device, framebufferCreateInfo);
    if (!framebuffer.has_value()) {
      std::cerr << "Failed to create framebuffer!" << std::endl;
      return std::nullopt;
    }
    framebuffers.push_back(std::move(framebuffer.value()));
  }

  return std::move(framebuffers);
}
