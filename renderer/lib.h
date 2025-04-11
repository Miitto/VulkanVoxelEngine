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
      : window(o.window), instance(o.instance), surface(o.surface),
        device(o.device), graphicsQueue(o.graphicsQueue),
        presentQueue(o.presentQueue), swapChain(o.swapChain) {
    o.moved = true;
  }
  App &operator=(App &&o) {
    window = o.window;
    instance = o.instance;
    surface = o.surface;
    device = o.device;
    graphicsQueue = o.graphicsQueue;
    presentQueue = o.presentQueue;
    swapChain = o.swapChain;
    o.moved = true;
    return *this;
  }
  ~App();

private:
  bool moved = false;
  Window window;
  Instance instance;
  Surface surface;
  LogicalDevice device;
  Queue graphicsQueue;
  Queue presentQueue;
  SwapChain swapChain;

  App(Window window, Instance instance, Surface surface, LogicalDevice device,
      Queue graphicsQueue, Queue presentQueue, SwapChain swapChain)
      : window(window), instance(instance), surface(surface), device(device),
        graphicsQueue(graphicsQueue), presentQueue(presentQueue),
        swapChain(swapChain) {}
};
