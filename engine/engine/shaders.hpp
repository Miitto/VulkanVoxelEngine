#pragma once

#include <expected>
#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace engine {

auto readFile(const std::string &filename)
    -> std::expected<std::vector<char>, std::string> {
    std::string path(SHADER_DIR + filename);
  std::ifstream file(path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return std::unexpected("Failed to open file: " + path);
  }

  std::vector<char> buffer(file.tellg());

  file.seekg(0);

  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

  file.close();

  return buffer;
}

[[nodiscard]]
auto createShaderModule(const vk::raii::Device &device,
                        const std::string &filename)
    -> std::expected<vk::raii::ShaderModule, std::string> {
  auto code_res = readFile(filename);

  if (!code_res) {
    return std::unexpected(code_res.error());
  }

  auto &code = code_res.value();

  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = code.size() * sizeof(char),
      .pCode = reinterpret_cast<const uint32_t *>(code.data())};

  auto shaderModule_res = device.createShaderModule(createInfo);

  if (!shaderModule_res) {
    return std::unexpected("Failed to create shader module: " +
                           vk::to_string(shaderModule_res.error()));
  }

  auto &shaderModule = shaderModule_res.value();

  return std::move(shaderModule);
}

class PipelineShaderStages {
public:
  vk::PipelineShaderStageCreateInfo vertex;
  vk::PipelineShaderStageCreateInfo fragment;

  [[nodiscard]] consteval auto size() const -> size_t { return 2; }
  [[nodiscard]] auto data() const -> const vk::PipelineShaderStageCreateInfo * {
    return &vertex;
  }
};

[[nodiscard]]
auto createPipelineShaderStages(const vk::raii::ShaderModule &module) {
  vk::PipelineShaderStageCreateInfo vertex{.stage =
                                               vk::ShaderStageFlagBits::eVertex,
                                           .module = *module,
                                           .pName = "vertMain"};

  vk::PipelineShaderStageCreateInfo fragment{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = *module,
      .pName = "fragMain"};

  return PipelineShaderStages{.vertex = vertex, .fragment = fragment};
}
} // namespace engine
