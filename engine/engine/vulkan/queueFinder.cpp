#include "engine/vulkan/queueFinder.hpp"

namespace engine::vulkan {

[[nodiscard]] auto
QueueFinder::find(const std::function<bool(QueueFamily)> &finder) const noexcept
    -> QueueFinder {
  auto filtered = std::vector<QueueFinder::QueueFamily>{};

  for (const auto &queueFamily : queueFamilyProperties) {
    if (finder(queueFamily)) {
      filtered.push_back(queueFamily);
    }
  }

  return {std::move(filtered)};
}

[[nodiscard]] auto QueueFinder::findType(const QueueType type) const noexcept
    -> QueueFinder {
  auto filtered = std::vector<QueueFinder::QueueFamily>{};

  for (auto &queueFamily : queueFamilyProperties) {
    switch (type.type) {
    case QueueTypeFlags::Graphics: {
      if (queueFamily.properties.queueFlags & vk::QueueFlagBits::eGraphics) {
        filtered.push_back(queueFamily);
      }
      break;
    }
    case QueueTypeFlags::Transfer: {
      if (queueFamily.properties.queueFlags & vk::QueueFlagBits::eTransfer) {
        filtered.push_back(queueFamily);
      }
      break;
    }
    case QueueTypeFlags::Present: {
      if (type.params.presentQueue.device.getSurfaceSupportKHR(
              queueFamily.index, type.params.presentQueue.surface)) {
        filtered.push_back(queueFamily);
      }
      break;
    }
    }
  }

  return {std::move(filtered)};
}

[[nodiscard]] auto
QueueFinder::findCombined(const std::vector<QueueType> &types) const noexcept
    -> QueueFinder {
  QueueFinder finder = *this;

  for (const auto &type : types) {
    finder = finder.findType(type);
  }

  return finder;
}
} // namespace engine::vulkan
