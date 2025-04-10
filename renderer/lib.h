#pragma once

#include "logicalDevice.h"
#include "queue.h"
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
        graphicsQueue(std::move(o.graphicsQueue)),
        presentQueue(std::move(o.presentQueue)) {
    o.moved = true;
  }
  App &operator=(App &&o) {
    window = std::move(o.window);
    instance = std::move(o.instance);
    surface = std::move(o.surface);
    device = std::move(o.device);
    graphicsQueue = std::move(o.graphicsQueue);
    presentQueue = std::move(o.presentQueue);
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

  App(Window window, Instance instance, Surface surface, LogicalDevice device,
      Queue graphicsQueue, Queue presentQueue)
      : window(std::move(window)), instance(std::move(instance)),
        surface(std::move(surface)), device(std::move(device)),
        graphicsQueue(std::move(graphicsQueue)),
        presentQueue(std::move(presentQueue)) {}
};
