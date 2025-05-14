#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vulkan/vulkan.h>

class Surface;
class Window;

class Instance {
public:
  class Ref {
    friend class Instance;
    std::shared_ptr<Instance *> m_instance;

    Ref() = delete;
    explicit Ref(Instance *instance)
        : m_instance(std::make_shared<Instance *>(instance)) {}

  protected:
    static Ref create(Instance &instance) { return Ref(&instance); }
    static Ref create(Instance *instance) { return Ref(instance); }

  public:
    void set(Instance *instance) { *this->m_instance = instance; }
    Instance &operator*() { return **m_instance; }
  };

private:
  VkInstance m_instance;
  Ref m_reference;

public:
  Instance(VkInstance instance)
      : m_instance(instance), m_reference(Ref::create(this)) {}
  ~Instance() {
    if (m_instance != VK_NULL_HANDLE) {
      vkDestroyInstance(m_instance, nullptr);
    }
  }

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;
  Instance &operator=(Instance &&o) = delete;
  Instance(Instance &&o) noexcept
      : m_instance(std::move(o.m_instance)), m_reference(o.m_reference) {
    o.m_instance = VK_NULL_HANDLE;
    m_reference.set(this);
  }

  VkInstance &operator*() { return m_instance; }
  Ref &ref() { return m_reference; }

  static std::optional<Instance> create(VkInstanceCreateInfo &createInfo);

  std::optional<Surface> createSurface(Window &window);
};
