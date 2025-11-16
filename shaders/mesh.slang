#include "include/camera.slang"

ConstantBuffer<Camera> camera;

struct Vertex {
  float3 position;
  float uvX;
  float3 normal;
  float uvY;
  float4 color;
};

struct Input {
    float4x4 model;
    Vertex* vertexBuffer;
};

[vk::push_constant]
uniform Input input;

struct VSOutput {
  float4 sv_position : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color;
};

[shader("vertex")]
VSOutput vert(uint index : SV_VertexID) {
    VSOutput output;

    Vertex v = input.vertexBuffer[index];

    float4 pos = float4(v.position, 1.0);

    float4 world = mul(input.model, pos);

    output.sv_position = camera.worldToClip(world);
    output.uv = float2(v.uvX, v.uvY);
    output.color = v.color;

    return output;
}

[shader("fragment")]
float4 frag(VSOutput inVert) : SV_Target {
    return inVert.color;
}
