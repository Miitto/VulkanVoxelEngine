#pragma once
#include "../instance.h"
#include "GLFW/glfw3.h"
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>

class InstanceCreateInfoBuilder {
  VkApplicationInfo appInfo;
  VkInstanceCreateInfo createInfo;
  bool validationLayers = false;
  bool GLFWExtensions = false;

  std::vector<const char *> layers;
  std::vector<const char *> extensions;

public:
  InstanceCreateInfoBuilder(VkApplicationInfo appInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;

    this->appInfo = appInfo;
  }

  InstanceCreateInfoBuilder &
  setApplicationInfo(const VkApplicationInfo appInfo) {
    this->appInfo = appInfo;
    createInfo.pApplicationInfo = &appInfo;
    return *this;
  }

  InstanceCreateInfoBuilder &addLayer(const char *layer) {
    layers.push_back(layer);
    return *this;
  }

  InstanceCreateInfoBuilder &addExtension(const char *extension) {
    extensions.push_back(extension);
    return *this;
  }

  InstanceCreateInfoBuilder &setFlags(VkInstanceCreateFlags flags) {
    createInfo.flags = flags;
    return *this;
  }

  InstanceCreateInfoBuilder &setNext(const void *next) {
    createInfo.pNext = next;
    return *this;
  }

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount == 0) {
      return false;
    }
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : layers) {
      bool layerFound = false;
      for (const auto &layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }
      if (!layerFound) {
        std::cerr << "Validation layer " << layerName
                  << " not found. Validation layers are not supported."
                  << std::endl;
        return false;
      }
    }
    return true;
  }

  const std::optional<VkInstanceCreateInfo> build() {
    if (GLFWExtensions) {
      uint32_t glfwExtensionCount = 0;
      const char **glfwExtensions =
          glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
      for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
      }
    }

    if (validationLayers) {
      layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    if (!layers.empty() && checkValidationLayerSupport()) {
      auto layerCount = layers.size();
      auto layerPtr = layerCount > 0 ? layers.data() : nullptr;

      createInfo.enabledLayerCount = layerCount;
      createInfo.ppEnabledLayerNames = layerPtr;
    }

    auto extensionCount = extensions.size();
    auto extensionPtr = extensionCount > 0 ? extensions.data() : nullptr;

    return createInfo;
  }

  std::optional<Instance> createInstance() {
    VkInstance instance;
    auto info = build();
    if (!info.has_value()) {
      return std::nullopt;
    }
    VkResult result = vkCreateInstance(&info.value(), nullptr, &instance);
    if (result != VK_SUCCESS) {
      return std::nullopt;
    }
    return Instance(instance);
  }

  InstanceCreateInfoBuilder &enableGLFWExtensions() {
    GLFWExtensions = true;
    return *this;
  }

  InstanceCreateInfoBuilder &enableValidationLayers() {
    validationLayers = true;
    return *this;
  }
};
