#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class InstancePtr {
  VkInstance instance;

public:
  InstancePtr(VkInstance instance) : instance(instance) {}
  ~InstancePtr() {
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
    }
  }

  InstancePtr(const InstancePtr &) = delete;
  InstancePtr &operator=(const InstancePtr &) = delete;

  InstancePtr(InstancePtr &&o) noexcept : instance(std::move(o.instance)) {
    o.instance = VK_NULL_HANDLE;
  }

  InstancePtr &operator=(InstancePtr &&o) noexcept {
    instance = std::move(o.instance);
    o.instance = VK_NULL_HANDLE;
    return *this;
  }

  VkInstance &operator*() { return instance; }
};

class Instance {
  std::shared_ptr<InstancePtr> instance;

  std::vector<std::shared_ptr<VkSurfaceKHR>> surfaces;

public:
  Instance(VkInstance instance)
      : instance(std::make_shared<InstancePtr>(instance)) {}

  VkInstance &operator*() { return **instance; }
};
