#pragma once

#include <glm/glm.hpp>

class Pipeline {
protected:
  vk::raii::PipelineLayout layout;
  vk::raii::Pipeline pipeline;

public:
  Pipeline(vk::raii::PipelineLayout &&layout,
           vk::raii::Pipeline &&pipeline) noexcept
      : layout(std::move(layout)), pipeline(std::move(pipeline)) {}
  auto operator*() noexcept -> vk::raii::Pipeline & { return pipeline; }

  auto operator*() const noexcept -> const vk::raii::Pipeline & {
    return pipeline;
  }

  operator vk::raii::Pipeline &() noexcept { return pipeline; }
  operator const vk::Pipeline &() const noexcept { return *pipeline; }

  [[nodiscard]]
  auto getLayout() noexcept -> vk::raii::PipelineLayout & {
    return layout;
  }
  [[nodiscard]] auto getLayout() const noexcept
      -> const vk::raii::PipelineLayout & {
    return layout;
  }
};

namespace pipelines {

class Mesh : public Pipeline {
public:
  struct Vertex {
    glm::vec3 position;
    float uvX;
    glm::vec3 normal;
    float uvY;
    glm::vec4 color;
  };

  struct MeshPushConstants {
    glm::mat4 modelMatrix;
    vk::DeviceAddress vBufferAddress;
  };

  struct DescriptorLayouts {
    const vk::raii::DescriptorSetLayout &camera;
  };

  static auto create(const vk::raii::Device &device, const vk::Format outFormat,
                     const DescriptorLayouts &layouts) noexcept
      -> std::expected<Mesh, std::string>;
};

} // namespace pipelines
