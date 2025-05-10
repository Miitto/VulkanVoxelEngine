#pragma once
#include "device/device.h"
#include "shaderModule.h"
#include <optional>
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

enum EShaderStage { VERTEX, FRAG, GEOM };

VkShaderStageFlagBits shaderStageToFlagBits(EShaderStage &stage);

class Shader {
  ShaderModule module;
  EShaderStage stage;

  Shader() = delete;
  Shader(const Shader &) = delete;

public:
  Shader(ShaderModule &module, EShaderStage stage)
      : module(std::move(module)), stage(stage) {}

  Shader(Shader &&other)
      : module(std::move(other.module)), stage(other.stage) {}

  EShaderStage getStage() { return stage; };
  VkShaderStageFlagBits getStageFlagBits() {
    return shaderStageToFlagBits(stage);
  }
  ShaderModule &getModule() { return module; }

  static std::optional<Shader> fromCode(std::vector<char> &code,
                                        EShaderStage stage, Device &device);
  static std::optional<Shader> fromFile(const std::string &fileName,
                                        EShaderStage stage, Device &device);
  static std::optional<Shader> fromModule(ShaderModule &module,
                                          EShaderStage stage);
};
