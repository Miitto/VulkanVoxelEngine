#pragma once

#include <vulkan/vulkan_core.h>

namespace vk {
namespace info {
class ImageViewCreate : public VkImageViewCreateInfo {
public:
  ImageViewCreate(VkImage image, VkFormat format)
      : VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                           .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1}} {}

  ImageViewCreate &setImage(VkImage img) {
    image = img;
    return *this;
  }

  ImageViewCreate &setViewType(VkImageViewType viewType) {
    this->viewType = viewType;
    return *this;
  }

  ImageViewCreate &setFormat(VkFormat format) {
    this->format = format;
    return *this;
  }

  ImageViewCreate &setComponents(VkComponentMapping components) {
    this->components = components;
    return *this;
  }

  ImageViewCreate &
  setSubresourceRange(VkImageSubresourceRange subresourceRange) {
    this->subresourceRange = subresourceRange;
    return *this;
  }
};
} // namespace info
} // namespace vk
