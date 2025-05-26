#include "semaphore.h"

namespace vk {
std::optional<Semaphore> Semaphore::create(Device &device) {
  VkSemaphoreCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };

  return create(device, createInfo);
}

std::optional<Semaphore> Semaphore::create(Device &device,
                                           VkSemaphoreCreateInfo createInfo) {
  VkSemaphore semaphore;

  auto result = vkCreateSemaphore(*device, &createInfo, nullptr, &semaphore);
  if (result != VK_SUCCESS) {
    return std::nullopt;
  }

  Semaphore sem(semaphore, device);

  return std::move(sem);
}
} // namespace vk
