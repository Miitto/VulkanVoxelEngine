#pragma once

#include <glm/glm.hpp>

namespace engine {
constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
constexpr glm::vec3 LEFT(-1.0f, 0.0f, 0.0f);
constexpr glm::vec3 RIGHT(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 FORWARD(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 BACKWARD(0.0f, 0.0f, 1.0f);
} // namespace engine
