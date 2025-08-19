#include "camera.hpp"
#include "defines.hpp"
#include "engine/vulkan/memorySelector.hpp"
#include "logger.hpp"
#include <intrin.h>

[[nodiscard]] auto PerspectiveCamera::createDescriptorSets(
    const vk::raii::Device &device,
    const vk::raii::DescriptorPool &descriptorPool,
    const vk::raii::DescriptorSetLayout &cameraLayout,
    std::array<vk::raii::Buffer, MAX_FRAMES_IN_FLIGHT> &cameraBuffers) noexcept
    -> std::expected<std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT>,
                     std::string> {
  std::array<vk::DescriptorSetLayout, _MM_FIXUP_MAX_FLOAT> layouts{
      *cameraLayout, *cameraLayout};
  vk::DescriptorSetAllocateInfo allocInfo{.descriptorPool = *descriptorPool,
                                          .descriptorSetCount =
                                              MAX_FRAMES_IN_FLIGHT,
                                          .pSetLayouts = layouts.data()};

  VK_MAKE(descriptorSet, device.allocateDescriptorSets(allocInfo),
          "Failed to allocate descriptor set");

  std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets{
      nullptr, nullptr};

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vk::DescriptorBufferInfo bufferInfo{.buffer = *cameraBuffers[i],
                                        .offset = 0,
                                        .range =
                                            sizeof(engine::Camera::Matrices)};

    vk::WriteDescriptorSet writeDescriptorSet{
        .dstSet = *descriptorSet[i],
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo,
    };

    device.updateDescriptorSets(writeDescriptorSet, {});
    descriptorSets[i] = std::move(descriptorSet[i]);
  }

  return std::move(descriptorSets);
}

auto PerspectiveCamera::createBuffers(
    const vk::raii::Device &device,
    const vk::raii::PhysicalDevice &physicalDevice,
    const vk::raii::DescriptorPool &cameraDescriptorPool,
    const vk::raii::DescriptorSetLayout &cameraLayout) noexcept
    -> std::expected<Buffers, std::string> {
  constexpr uint32_t BUF_SIZE = sizeof(engine::Camera::Matrices);
  constexpr uint32_t MEM_SIZE = BUF_SIZE * 2;
  std::array<vk::raii::Buffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers{nullptr,
                                                                    nullptr};

  VK_MAKE(buf1,
          device.createBuffer(vk::BufferCreateInfo{
              .size = sizeof(engine::Camera::Matrices),
              .usage = vk::BufferUsageFlagBits::eUniformBuffer,
              .sharingMode = vk::SharingMode::eExclusive}),
          "Failed to create uniform buffer 1");
  VK_MAKE(buf2,
          device.createBuffer(vk::BufferCreateInfo{
              .size = sizeof(engine::Camera::Matrices),
              .usage = vk::BufferUsageFlagBits::eUniformBuffer,
              .sharingMode = vk::SharingMode::eExclusive}),
          "Failed to create uniform buffer 2");

  engine::vulkan::MemorySelector memSelector(buf1, physicalDevice);
  EG_MAKE(allocInfo,
          memSelector.allocInfo(MEM_SIZE,
                                vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent),
          "Failed to get memory allocation info");

  VK_MAKE(uniformBufferMemory, device.allocateMemory(allocInfo),
          "Failed to allocate uniform buffer memory");

  uniformBuffers[0] = std::move(buf1);
  uniformBuffers[1] = std::move(buf2);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    uniformBuffers[i].bindMemory(*uniformBufferMemory, BUF_SIZE * i);
  }

  void *mapping = uniformBufferMemory.mapMemory(0, MEM_SIZE);

  EG_MAKE(cameraSets,
          PerspectiveCamera::createDescriptorSets(device, cameraDescriptorPool,
                                                  cameraLayout, uniformBuffers),
          "Failed to create descriptor sets for camera");

  return PerspectiveCamera::Buffers{.uniformBuffers = std::move(uniformBuffers),
                                    .uniformBufferMemory =
                                        std::move(uniformBufferMemory),
                                    .mapping = mapping,
                                    .descriptorSets = std::move(cameraSets)};
}

auto PerspectiveCamera::descriptorLayout(
    const vk::raii::Device &device) noexcept
    -> std::expected<vk::raii::DescriptorSetLayout, std::string> {
  vk::DescriptorSetLayoutBinding layoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
  };

  vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1,
                                               .pBindings = &layoutBinding};
  VK_MAKE(descriptorSetLayout, device.createDescriptorSetLayout(layoutInfo),
          "Failed to create descriptor set layout");

  return std::move(descriptorSetLayout);
}
