#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk {
class DescriptorSetLayoutBinding : public VkDescriptorSetLayoutBinding {
public:
  DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType type,
                             VkShaderStageFlags stageFlags, uint32_t count = 1)
      : VkDescriptorSetLayoutBinding{.binding = binding,
                                     .descriptorType = type,
                                     .descriptorCount = count,
                                     .stageFlags = stageFlags,
                                     .pImmutableSamplers = nullptr} {}
  DescriptorSetLayoutBinding(VkDescriptorSetLayoutBinding &&other)
      : VkDescriptorSetLayoutBinding(other) {}
};

namespace info {
class DescriptorSetLayoutCreate : public VkDescriptorSetLayoutCreateInfo {
  std::vector<DescriptorSetLayoutBinding> bindings;

  void setupBindings() {
    bindingCount = static_cast<uint32_t>(bindings.size());
    pBindings = bindings.data();
  }

public:
  DescriptorSetLayoutCreate()
      : VkDescriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = 0,
            .pBindings = nullptr} {}

  DescriptorSetLayoutCreate(VkDescriptorSetLayoutCreateInfo &&other)
      : VkDescriptorSetLayoutCreateInfo(other) {}

  DescriptorSetLayoutCreate &
  addBinding(const DescriptorSetLayoutBinding &binding) {
    bindings.push_back(binding);
    setupBindings();
    return *this;
  }
};
} // namespace info
} // namespace vk
