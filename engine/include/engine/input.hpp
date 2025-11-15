#pragma once

#include <engine/window.hpp>
#include <glm/glm.hpp>
#include <unordered_map>

namespace engine {
class Mouse {
  glm::vec2 m_position;
  glm::vec2 m_lastPosition;

public:
  Mouse() = default;

  [[nodiscard]] glm::vec2 pos() const { return m_position; }
  [[nodiscard]] glm::vec2 lastPos() const { return m_lastPosition; }

  [[nodiscard]] glm::vec2 delta() const { return m_position - m_lastPosition; }

  void move(glm::vec2 pos) { m_position = pos; }

  void onFrameEnd() { m_lastPosition = m_position; }
};

enum class KeyState : uint8_t { Pressed, Down, Released, Up };

class Input {
  std::unordered_map<engine::Key, KeyState> m_keyStates;
  Mouse m_mouse;

public:
  Input() = default;

  void setupWindow(engine::Window &window) {
    window.setKeyCallback([this](engine::Key key, int scancode,
                                 engine::KeyAction action, int mods) {
      (void)scancode;
      (void)mods;

      this->key(key, action);
    });

    window.setMouseMoveCallback([this](double xpos, double ypos) {
      m_mouse.move(
          glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos)));
    });
  }

  [[nodiscard]] const Mouse &mouse() const { return m_mouse; }
  [[nodiscard]] Mouse &mouseMut() { return m_mouse; }
  [[nodiscard]] const std::unordered_map<engine::Key, KeyState> &
  keyStates() const {
    return m_keyStates;
  }

  [[nodiscard]] bool isDown(engine::Key key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() &&
           (it->second == KeyState::Down || it->second == KeyState::Pressed);
  }

  [[nodiscard]] bool isPressed(engine::Key key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second == KeyState::Pressed;
  }

  [[nodiscard]] bool isReleased(engine::Key key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second == KeyState::Released;
  }

  [[nodiscard]] bool isUp(engine::Key key) const {
    auto it = m_keyStates.find(key);
    return it == m_keyStates.end();
  }

  void key(engine::Key key, engine::KeyAction action) {
    switch (action) {
    case engine::KeyAction::Press: {
      m_keyStates[key] = KeyState::Pressed;
      break;
    }
    case engine::KeyAction::Release: {
      m_keyStates[key] = KeyState::Released;
      break;
    }
    case engine::KeyAction::Repeat: {
      m_keyStates[key] = KeyState::Down;
      break;
    }
    }
  }

  void onFrameEnd() {
    m_mouse.onFrameEnd();

    std::vector<engine::Key> toErase;
    for (auto &[key, state] : m_keyStates) {
      if (state == KeyState::Pressed) {
        state = KeyState::Down;
      } else if (state == KeyState::Released) {
        toErase.push_back(key);
      }
    }

    for (auto key : toErase) {
      m_keyStates.erase(key);
    }
  }
};
} // namespace engine
