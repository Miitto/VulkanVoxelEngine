struct Camera {
  float4x4 view;
  float4x4 projection;
  float4x4 viewProjection;
  float4x4 invView;
  float4x4 invProjection;
  float4x4 invViewProjection;

  float4 worldToCam(float4 modelPos) {
    return mul(view, modelPos);
  }

  float4 camToClip(float4 worldPos) {
    return mul(projection, worldPos);
  }

  float4 worldToClip(float4 modelPos) {
    return mul(viewProjection, modelPos);
  }

  float4 camToWorld(float4 worldPos) {
    return mul(invView, worldPos);
  }

  float4 clipToCam(float4 clipPos) {
    return mul(invProjection, clipPos);
  }

  float4 clipToWorld(float4 clipPos) {
    return mul(invViewProjection, clipPos);
  }
};
