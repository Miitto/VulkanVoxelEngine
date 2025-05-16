#include "shader.h"
#include "shaderModule.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

static std::optional<std::vector<char>> readFile(const std::string &fileName) {
  std::ifstream file(fileName, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << fileName << std::endl;
    return std::nullopt;
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

std::optional<Shader> Shader::fromFile(const std::string &fileName,
                                       ShaderStage stage, Device &device) {
  auto code = readFile(fileName);
  if (!code) {
    std::cerr << "Failed to read shader file: " << fileName << std::endl;
    return std::nullopt;
  }

  return Shader::fromCode(*code, stage, device);
}

std::optional<Shader> Shader::fromCode(std::vector<char> &code,
                                       ShaderStage stage, Device &device) {

  auto module_opt = ShaderModule::fromCode(code, device);

  if (!module_opt.has_value())
    return std::nullopt;

  ShaderModule module = std::move(*module_opt);

  Shader shader(module, stage);

  return std::move(shader);
}
