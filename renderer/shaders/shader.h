#pragma once
#include "device/device.h"
#include "shaderModule.h"
#include <optional>
#include <string>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class ShaderStage {
public:
  enum Value {
    Vertex = VK_SHADER_STAGE_VERTEX_BIT,
    Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
    Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
    TessellationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TessellationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
  };

private:
  Value value;

  ShaderStage() = delete;

public:
  constexpr ShaderStage(Value value) : value(value) {}

  operator VkShaderStageFlagBits() const {
    return std::bit_cast<VkShaderStageFlagBits>(value);
  }
};

class Shader {
  ShaderModule module;
  ShaderStage stage;

  Shader() = delete;
  Shader(const Shader &) = delete;

public:
  Shader(ShaderModule &module, ShaderStage stage)
      : module(std::move(module)), stage(stage) {}

  Shader(Shader &&other) noexcept
      : module(std::move(other.module)), stage(other.stage) {}

  ShaderStage getStage() { return stage; };

  ShaderModule &getModule() { return module; }

  static std::optional<Shader> fromCode(std::vector<char> &code,
                                        ShaderStage stage, Device &device);
  static std::optional<Shader> fromFile(const std::string &fileName,
                                        ShaderStage stage, Device &device);
};
