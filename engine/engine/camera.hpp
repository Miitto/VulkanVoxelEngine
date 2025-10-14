#pragma once

#include "engine/directions.hpp"
#include "engine/logger.hpp"
#include "glm/ext/quaternion_geometric.hpp"
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

  Camera() : position(0.0f), rotation(1.0f, 0.0f, 0.0f, 0.0f) {}
  Camera(const glm::vec3 &position = glm::vec3(0.0f),
         const glm::quat &rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)) noexcept
      : position(position), rotation(rotation) {}

  [[nodiscard]] auto rotateVec(const glm::vec3 &vec) const noexcept
      -> glm::vec3 {
    return rotation * vec;
  }

public:
  auto rotate(const glm::quat &rotation) noexcept -> void {
    this->rotation = glm::normalize(this->rotation * rotation);
    Logger::debug("Camera rotated. New rotation: {}, {}, {}, {}",
                  this->rotation.w, this->rotation.x, this->rotation.y,
                  this->rotation.z);
  }

  void moveAbsolute(const glm::vec3 &delta) noexcept { position += delta; }

  void move(const glm::vec3 &axis) noexcept { position += rotateVec(axis); }

  void center() noexcept {
    position = glm::vec3(0.0f);
    rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  }

  virtual void onResize(uint32_t width, uint32_t height) noexcept = 0;

  [[nodiscard]] virtual auto view() const noexcept -> glm::mat4 = 0;
  [[nodiscard]] auto inverseView() const noexcept -> glm::mat4 {
    return glm::inverse(view());
  }

  [[nodiscard]] virtual auto projection() const noexcept -> glm::mat4 = 0;

  [[nodiscard]] virtual auto matrices() const noexcept -> Matrices {
    return Matrices{.view = view(), .projection = projection()};
  }

  [[nodiscard]] auto getPosition() const noexcept -> glm::vec3 {
    return position;
  }
  [[nodiscard]] auto getRotation() const noexcept -> glm::quat {
    return rotation;
  }

  virtual ~Camera() noexcept = default;
};
} // namespace engine
