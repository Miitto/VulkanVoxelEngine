#pragma once

#include <functional>
namespace engine::vulkan {

class QueueFinder {
public:
  struct QueueFamily {
    vk::QueueFamilyProperties properties;
    uint32_t index;
  };

private:
  std::vector<QueueFamily> queueFamilyProperties;

public:
  enum class QueueTypeFlags : uint8_t { Graphics, Transfer, Present };

  struct PresentQueue {
    const vk::raii::PhysicalDevice &device;
    const vk::raii::SurfaceKHR &surface;
  };

  union QueueTypeParams {
    void *none = nullptr;
    PresentQueue presentQueue;
  };

  struct QueueType {
    QueueTypeFlags type;
    QueueTypeParams params = QueueTypeParams{.none = nullptr};
  };

  QueueFinder(const vk::raii::PhysicalDevice &physicalDevice)
      : queueFamilyProperties(std::vector<QueueFamily>{}) {
    auto props = physicalDevice.getQueueFamilyProperties();

    queueFamilyProperties.reserve(props.size());

    for (uint32_t i = 0; i < props.size(); ++i) {
      queueFamilyProperties.push_back({props[i], i});
    }
  }

  QueueFinder(std::vector<QueueFamily> &&queueFamilyProperties)
      : queueFamilyProperties(std::move(queueFamilyProperties)) {}

  [[nodiscard]] auto find(const std::function<bool(QueueFamily)> &finder) const
      -> QueueFinder;
  [[nodiscard]] auto findType(const QueueType type) const -> QueueFinder;
  [[nodiscard]] auto findCombined(const std::vector<QueueType> &types) const
      -> QueueFinder;

  [[nodiscard]] auto queues() const -> const std::vector<QueueFamily> & {
    return queueFamilyProperties;
  }

  [[nodiscard]] auto hasQueue() const -> bool {
    return !queueFamilyProperties.empty();
  }

  [[nodiscard]] auto size() const -> size_t {
    return queueFamilyProperties.size();
  }

  [[nodiscard]] auto operator[](size_t index) const -> const QueueFamily & {
    return queueFamilyProperties[index];
  }

  [[nodiscard]] auto begin() const -> std::vector<QueueFamily>::const_iterator {
    return queueFamilyProperties.begin();
  }

  [[nodiscard]] auto end() const -> std::vector<QueueFamily>::const_iterator {
    return queueFamilyProperties.end();
  }

  [[nodiscard]] auto cbegin() const
      -> std::vector<QueueFamily>::const_iterator {
    return queueFamilyProperties.cbegin();
  }

  [[nodiscard]] auto cend() const -> std::vector<QueueFamily>::const_iterator {
    return queueFamilyProperties.cend();
  }

  [[nodiscard]] auto first() const -> const QueueFamily & {
    return queueFamilyProperties.front();
  }
};
} // namespace engine::vulkan
