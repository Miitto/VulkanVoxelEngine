#pragma once

#include "buffers/buffer.h"
#include "buffers/uniform.h"
#include "core.h"
#include "device/device.h"
#include "log.h"
#include <bit>
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

namespace enums {
enum class DescriptorType : uint32_t {
  Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
  CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
  SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
  StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
  UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
  StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
  UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
  StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
  UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
  InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
};
}

class DescriptorPoolSize : public VkDescriptorPoolSize {
public:
  DescriptorPoolSize(enums::DescriptorType type, uint32_t count)
      : VkDescriptorPoolSize{.type = std::bit_cast<VkDescriptorType>(type),
                             .descriptorCount = count} {}

  DescriptorPoolSize &setCount(uint32_t count) {
    descriptorCount = count;
    return *this;
  }

  DescriptorPoolSize &setType(enums::DescriptorType type) {
    this->type = std::bit_cast<VkDescriptorType>(type);
    return *this;
  }
};

namespace info {
class DescriptorPoolCreate : public VkDescriptorPoolCreateInfo {
  std::vector<DescriptorPoolSize> poolSizes{};

  void setupPoolSizes() {
    poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    pPoolSizes = poolSizes.data();
  }

public:
  DescriptorPoolCreate(uint32_t maxSets = 0)
      : VkDescriptorPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = maxSets,
            .poolSizeCount = 0,
            .pPoolSizes = nullptr} {}

  DescriptorPoolCreate(VkDescriptorPoolCreateInfo &&other)
      : VkDescriptorPoolCreateInfo(other) {}

  DescriptorPoolCreate &addPoolSize(const DescriptorPoolSize &poolSize) {
    poolSizes.push_back(poolSize);
    setupPoolSizes();
    return *this;
  }

  DescriptorPoolCreate &setMaxSets(uint32_t maxSets) {
    this->maxSets = maxSets;
    return *this;
  }
};
} // namespace info

class DescriptorSet;

class DescriptorPool {
  VkDescriptorPool m_handle;
  Device::Ref device;

  DescriptorPool(VkDescriptorPool handle, Device &device)
      : m_handle(handle), device(device.ref()) {}

  DescriptorPool() = delete;
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool &operator=(const DescriptorPool &) = delete;

public:
  DescriptorPool(DescriptorPool &&other) noexcept
      : m_handle(other.m_handle), device(std::move(other.device)) {
    other.m_handle = VK_NULL_HANDLE;
  }
  DescriptorPool &operator=(DescriptorPool &&other) noexcept {
    if (this != &other) {
      m_handle = other.m_handle;
      device = other.device;
      other.m_handle = VK_NULL_HANDLE;
    }
    return *this;
  }

  static std::optional<DescriptorPool> create(Device &device,
                                              info::DescriptorPoolCreate &info);
  ~DescriptorPool() {
    if (m_handle != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(*device, m_handle, nullptr);
      m_handle = VK_NULL_HANDLE;
    }
  }

  VkDescriptorPool operator*() { return m_handle; }
  operator VkDescriptorPool() { return m_handle; }

  std::optional<std::vector<DescriptorSet>>
  allocateSets(std::span<DescriptorSetLayout::Ref> &layouts);
  std::optional<std::vector<DescriptorSet>>
  allocateSets(DescriptorSetLayout &layout, uint32_t count = 1);
};

namespace info {
class DescriptorSetAllocate : public VkDescriptorSetAllocateInfo {
  std::vector<DescriptorSetLayout::Ref> layouts{};
  std::vector<VkDescriptorSetLayout> pLayouts{};

  void setupLayouts() {
    pLayouts.clear();
    pLayouts.reserve(layouts.size());

    for (const auto &layout : layouts) {
      pLayouts.push_back(*layout);
    }

    descriptorSetCount = static_cast<uint32_t>(pLayouts.size());
    pSetLayouts = pLayouts.data();
  }

public:
  DescriptorSetAllocate(VkDescriptorSetAllocateInfo &&other)
      : VkDescriptorSetAllocateInfo(other) {}
  DescriptorSetAllocate()
      : VkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = VK_NULL_HANDLE,
            .descriptorSetCount = 0,
            .pSetLayouts = nullptr} {}
  DescriptorSetAllocate(DescriptorPool &pool)
      : VkDescriptorSetAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = pool,
            .descriptorSetCount = 0,
            .pSetLayouts = nullptr} {
    setDescriptorPool(pool);
  }

  DescriptorSetAllocate &setDescriptorPool(DescriptorPool &pool) {
    descriptorPool = pool;
    return *this;
  }

  DescriptorSetAllocate &addLayout(DescriptorSetLayout &layout) {
    layouts.push_back(layout.ref());
    setupLayouts();
    return *this;
  }
  DescriptorSetAllocate &
  addLayouts(std::span<DescriptorSetLayout::Ref> &layoutSpan) {
    for (auto &layout : layoutSpan) {
      layouts.push_back(layout);
    }
    setupLayouts();
    return *this;
  }
};

class DescriptorBuffer : public VkDescriptorBufferInfo {
  VkDescriptorType m_bufferType;

public:
  DescriptorBuffer(Buffer buf, VkDescriptorType bufType,
                   VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
      : VkDescriptorBufferInfo{.buffer = buf, .offset = offset, .range = range},
        m_bufferType(bufType) {}

  DescriptorBuffer(UniformBuffer &buf, VkDeviceSize offset = 0,
                   VkDeviceSize range = VK_WHOLE_SIZE)
      : VkDescriptorBufferInfo{.buffer = buf, .offset = offset, .range = range},
        m_bufferType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {}

  VkDescriptorType bufferType() const { return m_bufferType; }
};

} // namespace info

class DescriptorSetWrite;

class DescriptorSet {
  VkDescriptorSet m_handle;
  Device::Ref m_device;

  DescriptorSet() = delete;

public:
  using Ref = RawRef<DescriptorSet, VkDescriptorSet>;
  explicit DescriptorSet(VkDescriptorSet set, Device &device)
      : m_handle(set), m_device(device.ref()) {}

  VkDescriptorSet operator*() const { return m_handle; }
  operator VkDescriptorSet() const { return m_handle; }

  void update(DescriptorSetWrite &write);
};

class DescriptorSetWrite : public VkWriteDescriptorSet {

public:
  DescriptorSetWrite(DescriptorSet &set, uint32_t binding)
      : VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                             .pNext = nullptr,
                             .dstSet = set,
                             .dstBinding = binding,
                             .dstArrayElement = 0,
                             .descriptorCount = 0,
                             .descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM,
                             .pImageInfo = nullptr,
                             .pBufferInfo = nullptr,
                             .pTexelBufferView = nullptr} {}
};

class DescriptorSetWriteBuffer : public DescriptorSetWrite {
  std::vector<info::DescriptorBuffer> m_buffers;

  void setupBuffers() {
    descriptorCount = static_cast<uint32_t>(m_buffers.size());
    pBufferInfo = m_buffers.data();
  }

public:
  DescriptorSetWriteBuffer(DescriptorSet &set, uint32_t binding)
      : DescriptorSetWrite(set, binding) {}

  DescriptorSetWriteBuffer(DescriptorSet &set, uint32_t binding,
                           UniformBuffer &buffer, uint32_t offset = 0,
                           VkDeviceSize range = VK_WHOLE_SIZE)
      : DescriptorSetWrite(set, binding) {
    m_buffers.push_back(info::DescriptorBuffer(buffer, offset, range));
    setupBuffers();
  }

  DescriptorSetWriteBuffer &addBuffer(info::DescriptorBuffer &buffer) {
    if (descriptorType == VK_DESCRIPTOR_TYPE_MAX_ENUM) {
      descriptorType = buffer.bufferType();
    } else if (descriptorType != buffer.bufferType()) {
      LOG_ERR("All buffers must have the same type.");
      return *this;
    }

    m_buffers.push_back(std::move(buffer));

    setupBuffers();
    return *this;
  }
};
} // namespace vk
