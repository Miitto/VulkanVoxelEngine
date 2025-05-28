module;

#include <array>

export module app:common;

import vk;

export const int MAX_FRAMES_IN_FLIGHT = 2;

export struct UObject {
  vk::UniformBuffer buffer;
  vk::MappingSegment bufferMapping;
  vk::DescriptorSet descriptorSet;
};

export struct UObjects {
  vk::DeviceMemory memory;
  vk::Mapping mapping;
  std::array<UObject, MAX_FRAMES_IN_FLIGHT> objects;
};

export struct Frame {
  vk::CommandBuffer commandBuffer;
  vk::Semaphore imageAvailable;
  vk::Semaphore renderFinished;
  vk::Fence inFlight;
};

export struct VBufferParts {
  vk::VertexBuffer vertexBuffer;
  vk::IndexBuffer indexBuffer;
  vk::DeviceMemory memory;
};
