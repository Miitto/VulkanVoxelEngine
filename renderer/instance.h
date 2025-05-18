#pragma once

#include "refs/refable.h"
#include "structs/info/instanceCreate.h"
#include <optional>
#include <utility>
#include <vulkan/vulkan.h>

class Surface;
class Window;

class Instance : public RawRefable<Instance, VkInstance> {
private:
  VkInstance m_instance;

public:
  using Ref = RawRef<Instance, VkInstance>;
  Instance(VkInstance instance) : RawRefable(), m_instance(instance) {}
  ~Instance() {
    if (m_instance != VK_NULL_HANDLE) {
      vkDestroyInstance(m_instance, nullptr);
    }
  }

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;
  Instance &operator=(Instance &&o) = delete;
  Instance(Instance &&o) noexcept
      : RawRefable(std::move(o)), m_instance(std::move(o.m_instance)) {
    o.m_instance = VK_NULL_HANDLE;
  }

  VkInstance &operator*() { return m_instance; }
  operator VkInstance() { return m_instance; }

  static std::optional<Instance> create(vk::info::InstanceCreate &createInfo);

  std::optional<Surface> createSurface(Window &window);
};
