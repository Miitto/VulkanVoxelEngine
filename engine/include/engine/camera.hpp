#pragma once

#include "engine/directions.hpp"
#include "engine/structs.hpp"
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

  struct Axes {
    float yaw;
    float pitch;
  };

  auto rotate(Axes axes) noexcept -> void {
    rotation.yaw += axes.yaw;
    rotation.pitch += axes.pitch;
  }

  virtual void update(const FrameData &) noexcept = 0;

  void moveAbsolute(const glm::vec3 &delta) noexcept { position += delta; }

  void move(const glm::vec3 &axis) noexcept { position += rotateVec(axis); }

  void center() noexcept {
    position = glm::vec3(0.0f);
    rotation = Axes{.yaw = 0.0f, .pitch = 0.0f};
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
  [[nodiscard]] auto getRotation() const noexcept -> Axes { return rotation; }

  virtual ~Camera() noexcept = default;

protected:
  glm::vec3 position;

  Axes rotation;

  Camera() : position(0.0f), rotation({.yaw = 0.0f, .pitch = 0.0f}) {}
  Camera(const glm::vec3 &position = glm::vec3(0.0f),
         const Axes rotation = {.yaw = 0.0f, .pitch = 0.0f}) noexcept
      : position(position), rotation(rotation) {}

  [[nodiscard]] glm::vec3 rotateVec(const glm::vec3 &v) const noexcept {
    auto yawRad = glm::radians(rotation.yaw);
    auto pitchRad = glm::radians(rotation.pitch);
    auto qYaw = glm::angleAxis(yawRad, glm::vec3(0.0f, 1.0f, 0.0f));
    auto qPitch = glm::angleAxis(pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));

    auto q = qYaw * qPitch;

    auto rotatedVec = q * glm::vec4(v, 0.0f);
    auto rotated = glm::vec3(rotatedVec);

    return rotated;
  }
};
} // namespace engine
