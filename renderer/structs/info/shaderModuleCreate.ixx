module;

#include "log.h"
#include <vector>
#include <vulkan/vulkan_core.h>

export module vk:info.shaderModuleCreate;

export namespace vk {
namespace info {

class ShaderModuleCreate : public VkShaderModuleCreateInfo {
  std::vector<char> m_code;

  void setupCode(const std::vector<char> &code) {

    if (!checkCode(code)) {
      LOG_ERR("Invalid shader code.");
      return;
    }

    m_code = code;
    codeSize = m_code.size();
    pCode = reinterpret_cast<const uint32_t *>(m_code.data());
  }

public:
  static bool checkCode(const std::vector<char> &code) {
    if (code.size() % sizeof(uint32_t) != 0) {
      LOG_ERR("Shader code size is not a multiple of 4 bytes.");
      return false;
    }

    const uint32_t *codePtr = reinterpret_cast<const uint32_t *>(code.data());

    if (codePtr[0] != 0x07230203) {
      LOG_ERR("Invalid magic number in shader code.");
      return false;
    }

    return true;
  }

  ShaderModuleCreate(const std::vector<char> code)
      : VkShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = 0,
            .pCode = nullptr} {
    setupCode(code);
  }

  ShaderModuleCreate &setCode(const std::vector<char> &code) {
    setupCode(code);

    return *this;
  }

  ShaderModuleCreate(const ShaderModuleCreate &other)
      : VkShaderModuleCreateInfo{other}, m_code(other.m_code) {
    setupCode(m_code);
  }

  ShaderModuleCreate(ShaderModuleCreate &&other) noexcept
      : VkShaderModuleCreateInfo{other}, m_code(std::move(other.m_code)) {
    setupCode(m_code);
    other.setupCode(m_code);
  }
};
} // namespace info
} // namespace vk
