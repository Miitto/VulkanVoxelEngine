#pragma once

#include <engine/window.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

namespace engine {

enum class KeyState : uint8_t { Down, Held, Up };

class Input {
public:
  class Mouse {
    glm::vec2 position;
    glm::vec2 lastPosition;

  public:
    Mouse() = default;

    [[nodiscard]] glm::vec2 pos() const { return position; }
    [[nodiscard]] glm::vec2 lastPos() const { return lastPosition; }

    [[nodiscard]] glm::vec2 delta() const { return position - lastPosition; }

    void move(glm::vec2 pos) { position = pos; }

    void onFrameEnd() { lastPosition = position; }

    friend class Input;
  };

protected:
  std::unordered_map<engine::Key, KeyState> keyState;
  Mouse m_mouse;
  engine::Window *window;

  Input() = default;

  static Input s_instance;

  bool m_imguiWantsKeyboard = false;
  bool m_imguiWantsMouse = false;

  void onKeyEvent(int key, int action);
  void onMouseMove(double x, double y);
  void onMouseButton(int button, int action);

  void imGuiWantsKeyboard(bool wants) { m_imguiWantsKeyboard = wants; }
  void imGuiWantsMouse(bool wants) { m_imguiWantsMouse = wants; }

public:
  inline static Input &instance() { return s_instance; }

  void setupWindow(engine::Window &w) {
    this->window = &w;
    w.setUserPtr(this);
    w.setKeyCallback(Input::glfwKeyCallback);
    w.setCursorPosCallback(Input::glfwCursorPosCallback);
    w.setMouseButtonCallback(Input::glfwMouseButtonCallback);
  }

  [[nodiscard]] const Mouse &mouse() const { return m_mouse; }
  [[nodiscard]] Mouse &mouseMut() { return m_mouse; }
  [[nodiscard]] const std::unordered_map<engine::Key, KeyState> &
  keyStates() const {
    return keyState;
  }

  [[nodiscard]]
  bool isPressed(engine::Key key) const {
    auto it = keyState.find(key);
    return it != keyState.end() &&
           (it->second == KeyState::Down || it->second == KeyState::Held);
  }

  [[nodiscard]] bool isDown(engine::Key key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Down;
  }

  [[nodiscard]] bool isHeld(engine::Key key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Held;
  }

  [[nodiscard]] bool isUp(engine::Key key) const {
    auto it = keyState.find(key);
    return it != keyState.end() && it->second == KeyState::Up;
  }

  [[nodiscard]] bool isReleased(engine::Key key) const {
    auto it = keyState.find(key);
    return it == keyState.end();
  }

  void key(engine::Key key, engine::KeyAction action) {
    switch (action) {
    case engine::KeyAction::Press: {
      keyState[key] = KeyState::Down;
      break;
    }
    case engine::KeyAction::Release: {
      keyState[key] = KeyState::Up;
      break;
    }
    case engine::KeyAction::Repeat: {
      keyState[key] = KeyState::Held;
      break;
    }
    }
  }

  void onFrameEnd() {
    m_mouse.onFrameEnd();

    std::vector<engine::Key> toErase;
    for (auto &[key, state] : keyState) {
      if (state == KeyState::Down) {
        state = KeyState::Held;
      } else if (state == KeyState::Up) {
        toErase.push_back(key);
      }
    }

    for (auto key : toErase) {
      keyState.erase(key);
    }
  }

  static void glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods);
  static void glfwCursorPosCallback(GLFWwindow *window, double xpos,
                                    double ypos);
  static void glfwMouseButtonCallback(GLFWwindow *window, int button,
                                      int action, int mods);
};
} // namespace engine
