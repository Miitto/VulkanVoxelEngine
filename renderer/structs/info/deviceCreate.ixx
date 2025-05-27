module;

#include <concepts>
#include <vector>
#include <vulkan/vulkan_core.h>

export module vk:info.deviceCreate;

export namespace vk::info {
class DeviceCreate : public VkDeviceCreateInfo {
  VkPhysicalDeviceFeatures m_features{};
  std::vector<vk::info::DeviceQueueCreate> m_queueCreateInfos{};
  std::vector<char const *> m_extensions{};

  void setupQueues() {
    queueCreateInfoCount = static_cast<uint32_t>(m_queueCreateInfos.size());
    pQueueCreateInfos = m_queueCreateInfos.data();
  }

  void setupExtensions() {
    enabledExtensionCount = static_cast<uint32_t>(m_extensions.size());
    ppEnabledExtensionNames = m_extensions.data();
  }

public:
  DeviceCreate()
      : VkDeviceCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                           .pNext = nullptr,
                           .flags = 0,
                           .queueCreateInfoCount = 0,
                           .pQueueCreateInfos = nullptr,
                           .enabledLayerCount = 0,
                           .ppEnabledLayerNames = nullptr,
                           .enabledExtensionCount = 0,
                           .ppEnabledExtensionNames = nullptr,
                           .pEnabledFeatures = nullptr} {}

  DeviceCreate(const VkPhysicalDeviceFeatures &features) : DeviceCreate() {
    m_features = features;
    pEnabledFeatures = &m_features;
  }

  DeviceCreate &
  setPhysicalDeviceFeatures(const VkPhysicalDeviceFeatures &features) {
    m_features = features;
    pEnabledFeatures = &m_features;

    return *this;
  }

  template <typename... Args>
    requires requires {
      !std::same_as<typename std::tuple_element_t<0, std::tuple<Args...>>::type,
                    vk::info::DeviceQueueCreate> &&
          !std::same_as<
              typename std::tuple_element_t<0, std::tuple<Args...>>::type,
              vk::info::DeviceQueueCreate &> &&
          std::constructible_from<DeviceQueueCreate, Args...>;
    }
  DeviceCreate &addQueue(Args &&...args) {
    DeviceQueueCreate queueCreateInfo(std::forward<Args>(args)...);
    return addQueue(std::move(queueCreateInfo));
  }

  DeviceCreate &addQueue(const DeviceQueueCreate &queueCreateInfo) {
    auto queueIdx = queueCreateInfo.queueFamilyIndex;

    for (auto &info : m_queueCreateInfos) {
      if (info.queueFamilyIndex == queueIdx) {
        return *this;
      }
    }

    m_queueCreateInfos.push_back(std::move(queueCreateInfo));

    setupQueues();
    return *this;
  }

  DeviceCreate &enableExtension(const char *extension) {
    m_extensions.push_back(extension);

    setupExtensions();

    return *this;
  }

  DeviceCreate &enableExtensions(const std::vector<char const *> &extensions) {
    m_extensions.insert(m_extensions.end(), extensions.begin(),
                        extensions.end());

    setupExtensions();

    return *this;
  }

  DeviceCreate(const DeviceCreate &other)
      : VkDeviceCreateInfo{other}, m_features(other.m_features),
        m_queueCreateInfos(other.m_queueCreateInfos),
        m_extensions(other.m_extensions) {
    setupQueues();
    setupExtensions();
  }

  DeviceCreate(DeviceCreate &&other) noexcept
      : VkDeviceCreateInfo{other}, m_features(std::move(other.m_features)),
        m_queueCreateInfos(std::move(other.m_queueCreateInfos)),
        m_extensions(std::move(other.m_extensions)) {
    pEnabledFeatures = &m_features;
    other.pEnabledFeatures = nullptr;
    setupQueues();
    setupExtensions();
    other.setupQueues();
    other.setupExtensions();
  }
};
} // namespace vk::info
