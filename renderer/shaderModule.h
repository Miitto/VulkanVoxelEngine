#pragma once
#include "logicalDevice.h"
#include <memory>
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

enum EShaderStage { VERTEX, FRAG, GEOM, TESS };

class ShaderModule {
  EShaderStage stage;
  std::shared_ptr<Device> device;
  VkShaderModule module;

public:
  ShaderModule(VkShaderModule module, std::shared_ptr<Device> device)
      : module(module), device(device) {}

  ShaderModule(const ShaderModule &) = delete;
  ShaderModule &operator=(const ShaderModule &) = delete;

  ShaderModule(ShaderModule &&other) noexcept
      : module(other.module), device(other.device) {
    other.module = VK_NULL_HANDLE;
  }

  ShaderModule &operator=(ShaderModule &&other) noexcept {
    if (this != &other) {
      module = other.module;
      device = other.device;
      other.module = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~ShaderModule() {
    if (module != VK_NULL_HANDLE) {
      vkDestroyShaderModule(**device, module, nullptr);
    }
  }

  VkShaderModule &operator*() { return module; }

public:
  EShaderStage getStage() const { return stage; }

  static std::optional<ShaderModule> fromCode(std::vector<char> &code,
                                              std::shared_ptr<Device> &device);
  static std::optional<ShaderModule> fromFile(const std::string &fileName,
                                              std::shared_ptr<Device> &device);
};
