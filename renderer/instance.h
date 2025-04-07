#pragma once

#include <vulkan/vulkan.h>

class Instance {
  VkInstance instance = nullptr;

public:
  Instance(VkInstance instance) : instance(instance) {}

  // Move only, and clean up old instance on move
  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;
  Instance(Instance &&o) noexcept : instance(o.instance) {
    o.instance = nullptr;
  };
  Instance &operator=(Instance &&o) noexcept {
    instance = o.instance;
    o.instance = nullptr;
    return *this;
  };

  ~Instance() {
    if (instance != nullptr) {
      vkDestroyInstance(instance, nullptr);
    }
  };

  VkInstance &operator*() { return instance; }
};
