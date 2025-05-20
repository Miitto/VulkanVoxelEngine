#version 460

layout(binding = 0) uniform TransformMatrices {
    mat4 model;
    mat4 view;
    mat4 projection;
} camera;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main() {
    fragColor = color;

    mat4 mvp = camera.projection * camera.view * camera.model;

    gl_Position = mvp * vec4(position, 1.0);
}
