#include <GLFW/glfw3.h>

class WindowManager {
public:
  WindowManager() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }

  void setResizable(bool resizable) {
    glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
  }

  ~WindowManager() { glfwTerminate(); }
};
