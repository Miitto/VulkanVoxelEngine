module;
#include <optional>
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>

export module vk:shader.mod;

import :device;

export namespace vk {
class ShaderModule {
  Device::Ref device;
  VkShaderModule module;

public:
  ShaderModule(VkShaderModule module, Device &device)
      : device(device.ref()), module(module) {}

  ShaderModule(const ShaderModule &) = delete;
  ShaderModule &operator=(const ShaderModule &) = delete;
  ShaderModule &operator=(ShaderModule &&other) = delete;

  ShaderModule(ShaderModule &&other) noexcept
      : device(other.device), module(other.module) {
    other.module = VK_NULL_HANDLE;
  }

  ~ShaderModule() {
    if (module != VK_NULL_HANDLE) {
      vkDestroyShaderModule(**device, module, nullptr);
    }
  }

  VkShaderModule &operator*() { return module; }

public:
  static std::optional<ShaderModule> fromCode(std::vector<char> &code,
                                              Device &device);
  static std::optional<ShaderModule> fromFile(const std::string &fileName,
                                              Device &device);
};
} // namespace vk
