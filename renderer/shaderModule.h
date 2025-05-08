#pragma once
#include "logicalDevice.h"
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

class ShaderModule {
  Device::Ref device;
  VkShaderModule module;

public:
  ShaderModule(VkShaderModule module, Device::Ref &device)
      : module(module), device(device) {}

  ShaderModule(const ShaderModule &) = delete;
  ShaderModule &operator=(const ShaderModule &) = delete;
  ShaderModule &operator=(ShaderModule &&other) = delete;

  ShaderModule(ShaderModule &&other) noexcept
      : module(other.module), device(other.device) {
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
                                              Device::Ref &device);
  static std::optional<ShaderModule> fromFile(const std::string &fileName,
                                              Device::Ref &device);
};
