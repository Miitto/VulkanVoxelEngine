struct VSInput {
  float2 position;
  float3 color;
};

struct VSOutput {
  float4 sv_position : SV_Position;
  float3 color;
};

[shader("vertex")]
VSOutput vert(VSInput input) {
    VSOutput output;

    output.sv_position = float4(input.position, 0.0, 1.0);
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 frag(VSOutput inVert) : SV_Target {
    float3 color = inVert.color;
    return float4(color, 1.0);
}
