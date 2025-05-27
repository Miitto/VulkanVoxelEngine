module;

#include <vector>
#include <vulkan/vulkan.h>

export module vk:info.pipeline.dynamicStateCreate;

export namespace vk {
namespace info {
class PipelineDynamicStateCreate : public VkPipelineDynamicStateCreateInfo {
  std::vector<VkDynamicState> dynamicStates;

  void setupDynamicStates() {
    dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    pDynamicStates = dynamicStates.data();
  }

public:
  PipelineDynamicStateCreate()
      : VkPipelineDynamicStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = 0,
            .pDynamicStates = nullptr} {}

  PipelineDynamicStateCreate &addDynamicState(VkDynamicState state) {
    dynamicStates.push_back(state);
    setupDynamicStates();
    return *this;
  }

  PipelineDynamicStateCreate &
  addDynamicStates(std::vector<VkDynamicState> states) {
    dynamicStates.insert(dynamicStates.end(), states.begin(), states.end());
    setupDynamicStates();
    return *this;
  }

  PipelineDynamicStateCreate &
  setDynamicStates(std::vector<VkDynamicState> states) {
    dynamicStates = std::move(states);
    setupDynamicStates();
    return *this;
  }

  PipelineDynamicStateCreate(const PipelineDynamicStateCreate &other)
      : VkPipelineDynamicStateCreateInfo{other},
        dynamicStates(other.dynamicStates) {
    setupDynamicStates();
  }

  PipelineDynamicStateCreate(PipelineDynamicStateCreate &&other) noexcept
      : VkPipelineDynamicStateCreateInfo{other},
        dynamicStates(std::move(other.dynamicStates)) {
    setupDynamicStates();
    other.setupDynamicStates();
  }
};
} // namespace info
} // namespace vk
