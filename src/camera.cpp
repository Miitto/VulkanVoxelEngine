#include "camera.hpp"
#include "engine/defines.hpp"
#include "logger.hpp"
#include <engine/directions.hpp>

void PerspectiveCamera::update(const engine::FrameData &data) noexcept {
  auto deltaMs = data.deltaTimeMs;
  auto scale = deltaMs * 25.0f;
  const auto &input = data.input;

  constexpr float rotationSpeed = 1.f;

  auto delta = input.mouse().delta();
  engine::Camera::Axes rot{
      .yaw = delta.x * rotationSpeed,
      .pitch = delta.y * rotationSpeed,
  };

  if (input.isPressed(engine::Key::W)) {
    move(engine::FORWARD * scale);
  }
  if (input.isPressed(engine::Key::S)) {
    move(engine::BACKWARD * scale);
  }
  if (input.isPressed(engine::Key::A)) {
    move(-engine::LEFT * scale);
  }
  if (input.isPressed(engine::Key::D)) {
    move(-engine::RIGHT * scale);
  }
  if (input.isPressed(engine::Key::Space)) {
    moveAbsolute(-engine::UP * scale);
  }
  if (input.isPressed(engine::Key::Ctrl)) {
    moveAbsolute(-engine::DOWN * scale);
  }

  if (input.isPressed(engine::Key::Left)) {
    rot.yaw -= scale;
  }
  if (input.isPressed(engine::Key::Right)) {
    rot.yaw += scale;
  }
  if (input.isPressed(engine::Key::Up)) {
    rot.pitch -= scale;
  }
  if (input.isPressed(engine::Key::Down)) {
    rot.pitch += scale;
  }

  if (input.isDown(engine::Key::C)) {
    center();
  }

  rotate(rot);
}

[[nodiscard]] auto PerspectiveCamera::createDescriptorSets(
    const vk::raii::Device &device,
    const vk::raii::DescriptorPool &descriptorPool,
    const vk::raii::DescriptorSetLayout &cameraLayout,
    std::array<vkh::AllocatedBuffer, MAX_FRAMES_IN_FLIGHT>
        &cameraBuffers) noexcept
    -> std::expected<std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT>,
                     std::string> {
  std::array<vk::DescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts{
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
    vk::DescriptorBufferInfo bufferInfo{.buffer = cameraBuffers[i].buffer,
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
    const vk::raii::Device &device, vma::Allocator &allocator,
    const vk::raii::DescriptorPool &cameraDescriptorPool,
    const vk::raii::DescriptorSetLayout &cameraLayout) noexcept
    -> std::expected<Buffers, std::string> {
  constexpr uint32_t BUF_SIZE = sizeof(engine::Camera::Matrices);
  std::array<vkh::AllocatedBuffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers{};

  vk::BufferCreateInfo bufferInfo{
      .size = BUF_SIZE,
      .usage = vk::BufferUsageFlagBits::eUniformBuffer,
      .sharingMode = vk::SharingMode::eExclusive,
  };

  vma::AllocationCreateInfo allocInfo{
      .flags = vma::AllocationCreateFlagBits::eMapped |
               vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
      .usage = vma::MemoryUsage::eCpuToGpu,
      .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                       vk::MemoryPropertyFlagBits::eHostCoherent,
  };

  EG_MAKE(buf1, vkh::AllocatedBuffer::create(allocator, bufferInfo, allocInfo),
          "Failed to create buffer 1");

  EG_MAKE(buf2, vkh::AllocatedBuffer::create(allocator, bufferInfo, allocInfo),
          "Failed to create buffer 2");

  uniformBuffers[0] = buf1;
  uniformBuffers[1] = buf2;

  EG_MAKE(cameraSets,
          PerspectiveCamera::createDescriptorSets(device, cameraDescriptorPool,
                                                  cameraLayout, uniformBuffers),
          "Failed to create descriptor sets for camera");

  return PerspectiveCamera::Buffers{.uniformBuffers = std::move(uniformBuffers),
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

void PerspectiveCamera::writeMatrices(const Buffers &buffers,
                                      uint32_t frame) const {
  auto m = matrices();

  memcpy(buffers.uniformBuffers[frame].allocInfo.pMappedData, &m,
         sizeof(engine::Camera::Matrices));
}
