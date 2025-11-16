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
    glm::mat4 viewProjection;
    glm::mat4 invView;
    glm::mat4 invProjection;
    glm::mat4 invViewProjection;
  };

  struct Axes {
    float yaw;
    float pitch;
  };

  auto rotate(Axes axes) noexcept -> void {
    rotation.yaw += axes.yaw;
    rotation.pitch += axes.pitch;

    if (rotation.pitch > 89.0f) {
      rotation.pitch = 89.0f;
    } else if (rotation.pitch < -89.0f) {
      rotation.pitch = -89.0f;
    }

    if (rotation.yaw > 360.0f) {
      rotation.yaw -= 360.0f;
    } else if (rotation.yaw < 0.0f) {
      rotation.yaw += 360.0f;
    }
  }

  virtual void update(const FrameData &) noexcept = 0;

  void moveAbsolute(const glm::vec3 &delta) noexcept { position += delta; }

  void move(const glm::vec3 &axis) noexcept { position += rotateVec(axis); }

  void center() noexcept {
    position = glm::vec3(0.0f);
    rotation = Axes{.yaw = 0.0f, .pitch = 0.0f};
  }

  virtual void onResize(uint32_t width, uint32_t height) noexcept = 0;

  glm::vec3 forward() const noexcept {
    glm::vec3 forward;
    forward.x = -std::sin(glm::radians(rotation.yaw)) *
                std::cos(glm::radians(rotation.pitch));
    forward.y = std::sin(glm::radians(rotation.pitch));
    forward.z = -std::cos(glm::radians(rotation.yaw)) *
                std::cos(glm::radians(rotation.pitch));
    return glm::normalize(forward);
  }

  [[nodiscard]] virtual auto view() const noexcept -> glm::mat4 = 0;
  [[nodiscard]] auto inverseView() const noexcept -> glm::mat4 {
    return glm::inverse(view());
  }

  [[nodiscard]] virtual auto projection() const noexcept -> glm::mat4 = 0;

  [[nodiscard]] virtual auto matrices() const noexcept -> Matrices {
    glm::mat4 v = view();
    glm::mat4 p = projection();

    glm::mat4 vp = p * v;

    glm::mat4 iv = glm::inverse(v);
    glm::mat4 ip = glm::inverse(p);
    glm::mat4 ivp = glm::inverse(vp);

    return Matrices{.view = v,
                    .projection = p,
                    .viewProjection = vp,
                    .invView = iv,
                    .invProjection = ip,
                    .invViewProjection = ivp};
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
