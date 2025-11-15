#pragma once

#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace vkh {
struct Queue {
  uint32_t index;
  std::shared_ptr<vk::raii::Queue> queue;
};

struct AllocatedImage {
  vk::Image image;
  vk::raii::ImageView view;
  vma::Allocation alloc;
  vk::Extent3D extent;
  vk::Format format;
};

struct AllocatedBuffer {
  vk::Buffer buffer;
  vma::Allocation alloc;
  vma::AllocationInfo allocInfo;
};

} // namespace vkh
