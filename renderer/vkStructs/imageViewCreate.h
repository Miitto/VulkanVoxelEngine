#include <vulkan/vulkan.h>

class ImageViewCreateInfoBuilder {
  VkImageViewCreateInfo createInfo;

public:
  ImageViewCreateInfoBuilder(VkImage image, VkFormat format) {
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.image = image;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.flags = 0;
  }

  ImageViewCreateInfoBuilder &setImage(VkImage image) {
    createInfo.image = image;
    return *this;
  }

  ImageViewCreateInfoBuilder &setViewType(VkImageViewType viewType) {
    createInfo.viewType = viewType;
    return *this;
  }

  ImageViewCreateInfoBuilder &setFormat(VkFormat format) {
    createInfo.format = format;
    return *this;
  }

  ImageViewCreateInfoBuilder &setComponents(VkComponentMapping components) {
    createInfo.components = components;
    return *this;
  }

  ImageViewCreateInfoBuilder &
  setSubresourceRange(VkImageSubresourceRange subresourceRange) {
    createInfo.subresourceRange = subresourceRange;
    return *this;
  }

  VkImageViewCreateInfo build() { return createInfo; }
};
