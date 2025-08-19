struct Camera {
  float4x4 view;
  float4x4 projection;

  float4 apply(float4 position) {
    return applyProj(applyView(position));
  }

  float4 applyView(float4 position) {
    return mul(view, position);
  }

  float4 applyProj(float4 position) {
    return mul(projection, position);
  }
};
