#pragma once
#include "logicalDevice.h"
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

enum EShaderStage { VERTEX, FRAG, GEOM, TESS };

class ShaderModulePtr {
  LogicalDevice device;
  VkShaderModule module;

public:
  ShaderModulePtr(VkShaderModule module, LogicalDevice device)
      : module(module), device(device) {}
  ShaderModulePtr(const ShaderModulePtr &) = delete;
  ShaderModulePtr &operator=(const ShaderModulePtr &) = delete;
  ShaderModulePtr(ShaderModulePtr &&other) noexcept
      : module(other.module), device(other.device) {
    other.module = VK_NULL_HANDLE;
  }
  ShaderModulePtr &operator=(ShaderModulePtr &&other) noexcept {
    if (this != &other) {
      module = other.module;
      device = other.device;
      other.module = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~ShaderModulePtr() {
    if (module != VK_NULL_HANDLE) {
      vkDestroyShaderModule(*device, module, nullptr);
    }
  }

  VkShaderModule &operator*() { return module; }
};

class ShaderModule {
  std::shared_ptr<ShaderModulePtr> shaderModulePtr;
  EShaderStage stage;

public:
  ShaderModule(VkShaderModule module, LogicalDevice device)
      : shaderModulePtr(std::make_shared<ShaderModulePtr>(module, device)) {}

  VkShaderModule &operator*() { return **shaderModulePtr; }
  EShaderStage getStage() const { return stage; }

  static std::optional<ShaderModule> fromCode(std::vector<char> &code,
                                              LogicalDevice &device);
  static std::optional<ShaderModule> fromFile(const std::string &fileName,
                                              LogicalDevice &device);
};
