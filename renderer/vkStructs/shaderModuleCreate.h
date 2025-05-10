#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <iostream>

class ShaderModuleCreateInfoBuilder {
  VkShaderModuleCreateInfo createInfo;
  std::vector<char> code;

public:
  bool checkCode(const std::vector<char> &code) {
    if (code.size() % sizeof(uint32_t) != 0) {
      std::cerr << "Shader code size is not a multiple of 4 bytes."
                << std::endl;
      return false;
    }

    const uint32_t *codePtr = reinterpret_cast<const uint32_t *>(code.data());

    if (codePtr[0] != 0x07230203) {
      std::cerr << "Invalid magic number in shader code." << std::endl;
      return false;
    }

    return true;
  }

  ShaderModuleCreateInfoBuilder(std::vector<char> code) {
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = 0;
    createInfo.pCode = nullptr;

    checkCode(code);
    this->code = code;
  }

  ShaderModuleCreateInfoBuilder &setCode(const std::vector<char> &code) {
    checkCode(code);
    this->code = code;

    return *this;
  }

  VkShaderModuleCreateInfo build() {
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    return createInfo;
  }
};
