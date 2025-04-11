#include "shader.h"
#include "vkStructs/shaderModuleCreate.h"
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

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
                                       LogicalDevice &device) {
  auto code = readFile(fileName);
  if (!code) {
    std::cerr << "Failed to read shader file: " << fileName << std::endl;
    return std::nullopt;
  }

  return Shader::fromCode(*code, device);
}

std::optional<Shader> Shader::fromCode(std::vector<char> &code,
                                       LogicalDevice &device) {
  auto createInfo = ShaderModuleCreateInfoBuilder(code).build();

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(*device, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create shader module" << std::endl;
    return std::nullopt;
  }

  ShaderModule module(shaderModule, device);
}
