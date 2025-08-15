#include <GLFW/glfw3.h>

class WindowManager {
public:
  WindowManager() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }

  ~WindowManager() { glfwTerminate(); }
};
