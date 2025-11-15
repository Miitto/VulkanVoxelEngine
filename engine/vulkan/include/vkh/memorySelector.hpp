#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace vkh {
class MemorySelector {
  vk::MemoryRequirements memReqs;
  vk::PhysicalDeviceMemoryProperties memProps;

public:
  MemorySelector(const vk::MemoryRequirements &requirements,
                 const vk::PhysicalDeviceMemoryProperties &properties) noexcept
      : memReqs(requirements), memProps(properties) {}

  MemorySelector(const vk::raii::Buffer &buffer,
                 const vk::raii::PhysicalDevice &physicalDevice) noexcept
      : memReqs(buffer.getMemoryRequirements()),
        memProps(physicalDevice.getMemoryProperties()) {}

  [[nodiscard]] auto
  selectMemoryType(const vk::MemoryPropertyFlags &properties) const noexcept
      -> std::expected<uint32_t, std::string> {
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
      if ((memReqs.memoryTypeBits & (1 << i)) &&
          (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    return std::unexpected(
        "Failed to find suitable memory type for the given requirements.");
  }

  [[nodiscard]] auto roundToAlign(const vk::DeviceSize size) const noexcept
      -> vk::DeviceSize {
    auto &alignment = memReqs.alignment;
    auto remainder = size % alignment;
    return remainder == 0 ? size : size + alignment - remainder;
  }

  [[nodiscard]] auto allocInfo(const vk::MemoryPropertyFlags &properties,
                               const uint32_t memSize) const noexcept
      -> std::expected<vk::MemoryAllocateInfo, std::string> {
    return allocInfo(memSize, properties);
  }

  [[nodiscard]] auto
  allocInfo(const vk::MemoryPropertyFlags &properties) const noexcept
      -> std::expected<vk::MemoryAllocateInfo, std::string> {
    return allocInfo(memReqs.size, properties);
  }

  [[nodiscard]] auto
  allocInfo(const vk::DeviceSize memSize,
            const vk::MemoryPropertyFlags &properties) const noexcept
      -> std::expected<vk::MemoryAllocateInfo, std::string> {
    auto memoryTypeIndex = selectMemoryType(properties);
    if (!memoryTypeIndex) {
      return std::unexpected(memoryTypeIndex.error());
    }

    vk::MemoryAllocateInfo allocInfo;
    allocInfo.allocationSize = roundToAlign(memSize);
    allocInfo.memoryTypeIndex = *memoryTypeIndex;

    return allocInfo;
  }

  [[nodiscard]] auto getMemoryRequirements() const noexcept
      -> const vk::MemoryRequirements & {
    return memReqs;
  }

  [[nodiscard]] auto getMemoryProperties() const noexcept
      -> const vk::PhysicalDeviceMemoryProperties & {
    return memProps;
  }
};
} // namespace vkh
