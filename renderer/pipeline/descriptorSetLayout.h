#pragma once

#include "device/device.h"
#include "structs/descriptorSetLayout.h"
#include <vulkan/vulkan_core.h>

namespace vk {
class DescriptorSetLayout
    : public RawRefable<DescriptorSetLayout, VkDescriptorSetLayout> {
  VkDescriptorSetLayout m_layout;
  Device::Ref m_device;

  DescriptorSetLayout(VkDescriptorSetLayout &layout, Device &device)
      : RawRefable(), m_layout(layout), m_device(device.ref()) {}

  DescriptorSetLayout() = delete;
  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

public:
  using Ref = RawRef<DescriptorSetLayout, VkDescriptorSetLayout>;
  DescriptorSetLayout(DescriptorSetLayout &&other) noexcept
      : RawRefable(std::move(other)), m_layout(other.m_layout),
        m_device(other.m_device) {
    other.m_layout = VK_NULL_HANDLE;
  }

  static std::optional<DescriptorSetLayout>
  create(Device &device, info::DescriptorSetLayoutCreate &createInfo);
  ~DescriptorSetLayout() {
    if (m_layout != VK_NULL_HANDLE) {
      vkDestroyDescriptorSetLayout(*m_device, m_layout, nullptr);
      m_layout = VK_NULL_HANDLE;
    }
  }

  VkDescriptorSetLayout operator*() { return m_layout; }
  operator VkDescriptorSetLayout() { return m_layout; }

  Device::Ref device() { return m_device; }
};
} // namespace vk
