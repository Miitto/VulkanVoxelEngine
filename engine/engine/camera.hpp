#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine {
class Camera {
public:
  struct Matrices {
    glm::mat4 view;
    glm::mat4 projection;
  };

protected:
  glm::vec3 position;
  glm::quat rotation;

  Camera(const glm::vec3 &position = glm::vec3(0.0f),
         const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
      : position(position), rotation(rotation) {}

public:
  auto rotate(const glm::quat &rotation) -> void {
    this->rotation = rotation * this->rotation;
  }

  virtual void onResize(uint32_t width, uint32_t height) = 0;

  [[nodiscard]] virtual auto view() const -> glm::mat4 = 0;
  [[nodiscard]] auto inverseView() const -> glm::mat4 {
    return glm::inverse(view());
  }

  [[nodiscard]] virtual auto projection() const -> glm::mat4 = 0;

  [[nodiscard]] virtual auto matrices() const -> Matrices {
    return Matrices{.view = view(), .projection = projection()};
  }

  [[nodiscard]] auto getPosition() const -> glm::vec3 { return position; }
  [[nodiscard]] auto getRotation() const -> glm::quat { return rotation; }

  virtual ~Camera() = default;
};
} // namespace engine
