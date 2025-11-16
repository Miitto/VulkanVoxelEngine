#include <engine/input.hpp>

#include "logger.hpp"

namespace engine {
Input Input::s_instance = {};

void Input::glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                            int action, int mods) {
  Input *input = reinterpret_cast<Input *>(engine::Window::getUserPtr(window));
  input->onKeyEvent(key, action);

  (void)scancode;
  (void)mods;
}

void Input::glfwCursorPosCallback(GLFWwindow *window, double xpos,
                                  double ypos) {
  Input *input = reinterpret_cast<Input *>(engine::Window::getUserPtr(window));
  input->onMouseMove(xpos, ypos);
}

void Input::glfwMouseButtonCallback(GLFWwindow *window, int button, int action,
                                    int mods) {
  Input *input = reinterpret_cast<Input *>(engine::Window::getUserPtr(window));
  (void)mods;
  input->onMouseButton(button, action);
}

void Input::onKeyEvent(int key, int action) {
  if (m_imguiWantsKeyboard && action != GLFW_RELEASE) {
    return;
  }
  KeyState state = action == GLFW_PRESS    ? KeyState::Down
                   : action == GLFW_REPEAT ? KeyState::Held
                                           : KeyState::Up;
  keyState[static_cast<Key>(key)] = state;
}
void Input::onMouseMove(double x, double y) {
  if (m_imguiWantsMouse) {
    // If ImGui wants the mouse, don't update delta (still want position for
    // accurate tracking)
    m_mouse.position = {x, y};
    return;
  }
  m_mouse.position = {x, y};
}

void Input::onMouseButton(int button, int action) {
  (void)button;
  if (m_imguiWantsMouse && action != GLFW_RELEASE) {
    return;
  }

  Logger::debug("Clicked at {}, {}", m_mouse.position.x, m_mouse.position.y);
}
} // namespace engine