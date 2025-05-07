#pragma once

#include "logicalDevice.h"
#include "queue.h"
#include "swapChain.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "instance.h"
#include "surface.h"
#include "window.h"

class App {
public:
  static std::optional<App> create();
  void run();

  // Since we have move-only members, we also need to be move-only
  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&o)
      : window(std::move(o.window)), instance(std::move(o.instance)),
        surface(std::move(o.surface)), device(std::move(o.device)),
        graphicsQueue(o.graphicsQueue), presentQueue(o.presentQueue),
        swapChain(std::move(o.swapChain)) {
    o.moved = true;
  }
  App &operator=(App &&o) = delete;
  ~App();

private:
  bool moved = false;
  Window window;
  Instance instance;
  Surface surface;
  Device device;
  Queue graphicsQueue;
  Queue presentQueue;
  SwapChain swapChain;

  App(Window window, Instance instance, Surface surface, Device device,
      Queue graphicsQueue, Queue presentQueue, SwapChain swapChain)
      : window(std::move(window)), instance(std::move(instance)),
        surface(std::move(surface)), device(std::move(device)),
        graphicsQueue(graphicsQueue), presentQueue(presentQueue),
        swapChain(std::move(swapChain)) {}
};
