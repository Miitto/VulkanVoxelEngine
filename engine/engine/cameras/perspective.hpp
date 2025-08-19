#pragma once

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
    float farPlane;
  };

private:
  Params params;

public:
  Perspective(const glm::vec3 &position, const glm::quat &rotation,
              const Params &params)
      : engine::Camera(position, rotation), params(params) {}

  void onResize(uint32_t width, uint32_t height) override {
    params.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
  }

  [[nodiscard]] auto view() const -> glm::mat4 override {
    auto forward = glm::normalize(
        glm::vec3(rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    return glm::lookAt(position, position + forward,
                       glm::vec3(0.0f, 1.0f, 0.0f));
  }

  [[nodiscard]] auto projection() const -> glm::mat4 override {
    auto proj = glm::perspective(params.fov, params.aspectRatio,
                                 params.nearPlane, params.farPlane);
    proj[1][1] *= -1; // Flip Y for Vulkan
    return proj;
  }
};
} // namespace engine::cameras
