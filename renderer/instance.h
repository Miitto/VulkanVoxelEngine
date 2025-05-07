#pragma once

#include <memory>
#include <utility>
#include <vulkan/vulkan.h>

class Instance {
public:
  class Ref {
    friend class Instance;
    std::shared_ptr<Instance *> instance;

    Ref() = delete;
    explicit Ref(Instance *instance)
        : instance(std::make_shared<Instance *>(instance)) {}

  protected:
    static Ref create(Instance &instance) { return Ref(&instance); }
    static Ref create(Instance *instance) { return Ref(instance); }

  public:
    void set(Instance *instance) { *this->instance = instance; }
    Instance &operator*() { return **instance; }
  };

private:
  VkInstance instance;
  Ref reference;

public:
  Instance(VkInstance instance)
      : instance(instance), reference(Ref::create(this)) {}
  ~Instance() {
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
    }
  }

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;
  Instance &operator=(Instance &&o) = delete;
  Instance(Instance &&o) noexcept
      : instance(std::move(o.instance)), reference(o.reference) {
    o.instance = VK_NULL_HANDLE;
    reference.set(this);
  }

  VkInstance &operator*() { return instance; }
  Ref &ref() { return reference; }
};
