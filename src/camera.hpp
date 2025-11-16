#pragma once

#include "engine/defines.hpp"
#include "vkh/structs.hpp"
#include <engine/cameras/perspective.hpp>
#include <engine/util/macros.hpp>
#include <expected>
#include <string>
#include <vulkan/vulkan_raii.hpp>

class PerspectiveCamera : public engine::cameras::Perspective {
public:
  struct Buffers {
    std::array<vkh::AllocatedBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
    std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
  };

  PerspectiveCamera(const glm::vec3 &position,
                    const engine::Camera::Axes &rotation,
                    const engine::cameras::Perspective::Params &params) noexcept
      : engine::cameras::Perspective(position, rotation, params) {}

  void update(const engine::FrameData &) noexcept override;

  [[nodiscard]] static auto
  descriptorLayout(const vk::raii::Device &device) noexcept
      -> std::expected<vk::raii::DescriptorSetLayout, std::string>;

  [[nodiscard]] static auto
  createDescriptorSets(const vk::raii::Device &device,
                       const vk::raii::DescriptorPool &descriptorPool,
                       const vk::raii::DescriptorSetLayout &cameraLayout,
                       std::array<vkh::AllocatedBuffer, MAX_FRAMES_IN_FLIGHT>
                           &cameraBuffers) noexcept
      -> std::expected<
          std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT>,
          std::string>;

  [[nodiscard]] static auto
  createBuffers(const vk::raii::Device &device, vma::Allocator &allocator,
                const vk::raii::DescriptorPool &cameraDescriptorPool,
                const vk::raii::DescriptorSetLayout &cameraLayout) noexcept
      -> std::expected<Buffers, std::string>;

protected:
  void writeMatrices() const {
    auto m = matrices();
    memcpy(buffer.allocInfo.pMappedData, &m, sizeof(engine::Camera::Matrices));
  }

  vkh::AllocatedBuffer buffer;
};
