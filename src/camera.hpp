#pragma once

#include "defines.hpp"
#include <engine/cameras/perspective.hpp>
#include <engine/util/macros.hpp>
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

class PerspectiveCamera : public engine::cameras::Perspective {

public:
  struct Buffers {
    std::array<vk::raii::Buffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
    vk::raii::DeviceMemory uniformBufferMemory;
    void *mapping;
    std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
  };

  PerspectiveCamera(const glm::vec3 &position, const glm::quat &rotation,
                    const engine::cameras::Perspective::Params &params)
      : engine::cameras::Perspective(position, rotation, params) {}

  [[nodiscard]] static auto descriptorLayout(const vk::raii::Device &device)
      -> std::expected<vk::raii::DescriptorSetLayout, std::string>;

  [[nodiscard]] static auto createDescriptorSets(
      const vk::raii::Device &device,
      const vk::raii::DescriptorPool &descriptorPool,
      const vk::raii::DescriptorSetLayout &cameraLayout,
      std::array<vk::raii::Buffer, MAX_FRAMES_IN_FLIGHT> &cameraBuffers)
      -> std::expected<
          std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT>,
          std::string>;

  [[nodiscard]] static auto
  createBuffers(const vk::raii::Device &device,
                const vk::raii::PhysicalDevice &physicalDevice,
                const vk::raii::DescriptorPool &cameraDescriptorPool,
                const vk::raii::DescriptorSetLayout &cameraLayout)
      -> std::expected<Buffers, std::string>;
};
