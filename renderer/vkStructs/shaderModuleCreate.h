#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class ShaderModuleCreateInfoBuilder {
  VkShaderModuleCreateInfo createInfo;
  std::vector<char> code;

public:
  ShaderModuleCreateInfoBuilder(std::vector<char> code) {
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  }

  ShaderModuleCreateInfoBuilder &setCode(const std::vector<char> &newCode) {
    code = newCode;
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
    createInfo.codeSize = code.size();
    return *this;
  }

  VkShaderModuleCreateInfo &build() { return createInfo; }
};
