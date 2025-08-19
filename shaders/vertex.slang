#include "include/camera.slang"

ConstantBuffer<Camera> camera;

struct VSInput {
  float3 position;
  float3 color;
};

struct VSOutput {
  float4 sv_position : SV_Position;
  float4 color;
};

[shader("vertex")]
VSOutput vert(VSInput input) {
    VSOutput output;

    output.sv_position = camera.apply(float4(input.position, 1.0));
    output.color = float4(input.color, 1.0);

    return output;
}

[shader("fragment")]
float4 frag(VSOutput inVert) : SV_Target {
    return inVert.color;
}
