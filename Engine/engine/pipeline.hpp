#pragma once

#include <array>
#include <vulkan/vulkan_raii.hpp>

namespace engine {

template <typename... Args> class DynamicStateInfo {
  std::array<vk::DynamicState, sizeof...(Args)> dynamicStates;
  vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;

public:
  constexpr DynamicStateInfo(Args... args) : dynamicStates{args...} {
    dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()};
  }

  operator vk::PipelineDynamicStateCreateInfo() const {
    return dynamicStateCreateInfo;
  }

  operator vk::PipelineDynamicStateCreateInfo *() {
    return &dynamicStateCreateInfo;
  }
};
} // namespace engine
