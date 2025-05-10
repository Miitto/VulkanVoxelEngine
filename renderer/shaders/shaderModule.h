#pragma once
#include "device/device.h"
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

class ShaderModule {
  Device::Ref device;
  VkShaderModule module;

public:
  ShaderModule(VkShaderModule module, Device::Ref &device)
      : device(device), module(module) {}

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
