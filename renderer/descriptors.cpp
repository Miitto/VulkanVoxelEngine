#include "descriptors.h"

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

std::optional<vk::DescriptorPool>
vk::DescriptorPool::create(Device &device, info::DescriptorPoolCreate &info) {
  VkDescriptorPool pool;
  if (vkCreateDescriptorPool(*device, &info, nullptr, &pool) != VK_SUCCESS) {
    return std::nullopt;
  }
  return DescriptorPool(pool, device);
}

std::optional<std::vector<vk::DescriptorSet>>
vk::DescriptorPool::allocateSets(DescriptorSetLayout &layout, uint32_t count) {
  auto ref = layout.ref();
  std::vector<DescriptorSetLayout::Ref> layoutsVec(count, ref);
  std::span<DescriptorSetLayout::Ref> layoutsSpan(layoutsVec);

  return allocateSets(layoutsSpan);
}

std::optional<std::vector<vk::DescriptorSet>>
vk::DescriptorPool::allocateSets(std::span<DescriptorSetLayout::Ref> &layouts) {
  if (layouts.empty()) {
    return std::nullopt;
  }

  info::DescriptorSetAllocate allocInfo(*this);
  allocInfo.addLayouts(layouts);

  std::vector<VkDescriptorSet> sets;
  sets.resize(layouts.size());

  if (vkAllocateDescriptorSets(device, &allocInfo, sets.data()) != VK_SUCCESS) {
    LOG_ERR("Failed to allocate descriptor sets.");
    return std::nullopt;
  }

  std::vector<DescriptorSet> descriptorSets;
  descriptorSets.reserve(sets.size());
  for (auto &set : sets) {
    descriptorSets.emplace_back(set, device);
  }

  return descriptorSets;
}

void vk::DescriptorSet::update(vk::DescriptorSetWrite &write) {
  vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}
