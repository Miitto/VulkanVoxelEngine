#pragma once
#include "logicalDevice.h"
#include <memory>
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

class Shader {

public:
  static std::optional<Shader> fromCode(std::vector<char> &code,
                                        std::shared_ptr<Device> &device);
  static std::optional<Shader> fromFile(const std::string &fileName,
                                        std::shared_ptr<Device> &device);
};
