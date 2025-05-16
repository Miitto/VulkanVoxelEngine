#pragma once

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"
#include <cstring>
#include <iostream>
#include <optional>
#include <print>
#include <vector>

namespace vk {
namespace info {
class InstanceCreate : public VkInstanceCreateInfo {
  std::vector<const char *> layers;
  std::vector<const char *> extensions;

  std::optional<const char *> checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount == 0) {
      return std::nullopt;
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
        std::cerr << "Validation layer " << layerName << " not found."
                  << std::endl;
        return layerName;
      }
    }
    return std::nullopt;
  }

  std::optional<const char *> checkLayers() {
    if (layers.empty()) {
      return std::nullopt;
    }

    return checkValidationLayerSupport();
  }

  void setupExtensions() {
    enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ppEnabledExtensionNames = extensions.data();
  }

  void setupLayers() {
    auto check = checkLayers();

    while (check.has_value()) {
      std::cerr << "Removing invalid layer: " << *check << std::endl;

      for (size_t i = 0; i < layers.size(); i++) {
        if (strcmp(layers[i], *check) == 0) {
          layers.erase(layers.begin() + i);
          break;
        }
      }

      check = checkLayers();
    }

    enabledLayerCount = static_cast<uint32_t>(layers.size());
    ppEnabledLayerNames = layers.data();
  }

public:
  enum class ValidationType { NONE, DEBUG, ALWAYS };
  InstanceCreate(VkApplicationInfo &appInfo,
                 ValidationType validation = ValidationType::DEBUG,
                 bool glfwExtensions = true)
      : VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = 0,
            .ppEnabledExtensionNames = nullptr,
        } {

    switch (validation) {
    case ValidationType::DEBUG: {
#ifndef NDEBUG
      enableValidationLayers();
#endif
      break;
    }
    case ValidationType::ALWAYS: {
      enableValidationLayers();
      break;
    }
    default: {
      break;
    }
    }

    if (glfwExtensions) {
      enableGLFWExtensions();
    }
  }

  InstanceCreate &addLayer(const char *layer) {
    layers.push_back(layer);

    setupLayers();
    return *this;
  }

  InstanceCreate &addExtension(const char *extension) {
    extensions.push_back(extension);

    setupExtensions();
    return *this;
  }

  InstanceCreate &enableGLFWExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<char *> glfwExt(glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      glfwExt[i] = const_cast<char *>(glfwExtensions[i]);
    }
    std::print("Enabling {} GLFW extensions (", glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      std::print("{}", glfwExtensions[i]);
      if (i != glfwExtensionCount - 1) {
        std::print(", ");
      }
    }
    std::print(")\n");
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      extensions.push_back(glfwExtensions[i]);
    }

    setupExtensions();

    return *this;
  }

  InstanceCreate &enableValidationLayers() {
    layers.push_back("VK_LAYER_KHRONOS_validation");

    setupLayers();

    return *this;
  }

  InstanceCreate(const InstanceCreate &other)
      : VkInstanceCreateInfo{other}, layers(other.layers),
        extensions(other.extensions) {
    setupLayers();
    setupExtensions();
  }

  InstanceCreate(InstanceCreate &&other) noexcept
      : VkInstanceCreateInfo{other}, layers(std::move(other.layers)),
        extensions(std::move(other.extensions)) {
    setupLayers();
    setupExtensions();
    other.setupLayers();
    other.setupExtensions();
  }
};
} // namespace info
} // namespace vk
