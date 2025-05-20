#include "descriptorSetLayout.h"

std::optional<vk::DescriptorSetLayout>
vk::DescriptorSetLayout::create(Device &device,
                                info::DescriptorSetLayoutCreate &createInfo) {
  VkDescriptorSetLayout layout;
  if (vkCreateDescriptorSetLayout(*device, &createInfo, nullptr, &layout) !=
      VK_SUCCESS) {
    return std::nullopt;
  }

  return DescriptorSetLayout(layout, device);
}
