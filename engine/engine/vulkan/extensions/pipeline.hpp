#pragma once

#include <array>
#include <vulkan/vulkan_raii.hpp>

namespace engine::vulkan {

template <typename... Args> class DynamicStateInfo {
  std::array<vk::DynamicState, sizeof...(Args)> dynamicStates;
  vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;

public:
  constexpr DynamicStateInfo(Args... args) noexcept : dynamicStates{args...} {
    dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};
  }

  operator vk::PipelineDynamicStateCreateInfo() const noexcept {
    return dynamicStateCreateInfo;
  }

  operator vk::PipelineDynamicStateCreateInfo *() noexcept {
    return &dynamicStateCreateInfo;
  }
};
} // namespace engine::vulkan
