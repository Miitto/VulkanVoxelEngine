#pragma once

#include "engine/directions.hpp"
#include <engine/camera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::cameras {
class Perspective : public engine::Camera {
public:
  struct Params {
    /// Field of view in radians
    float fov;
    float aspectRatio;
    float nearPlane;
  };

private:
  Params params;

public:
  Perspective(const glm::vec3 &position, const Camera::Axes &rotation,
              const Params &params) noexcept
      : engine::Camera(position, rotation), params(params) {}

  virtual void update(const FrameData &) noexcept override = 0;

  void onResize(uint32_t width, uint32_t height) noexcept override {
    params.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
  }

  [[nodiscard]] auto view() const noexcept -> glm::mat4 override {
    return glm::lookAt(position, position + forward(), engine::UP);
  }

  [[nodiscard]] auto projection() const noexcept -> glm::mat4 override {
    glm::mat4 proj = glm::zero<glm::mat4>();
    float f = 1.0f / tan(params.fov / 2.0f);

    proj[0][0] = f / params.aspectRatio;
    proj[1][1] = f;
    proj[2][3] = 1.0f;
    proj[3][2] = params.nearPlane;

    return proj;
  }
};
} // namespace engine::cameras
