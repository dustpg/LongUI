// 2D纹理 第一个输入储存在t0
Texture2D InputTexture : register(t0);
// 采样器状态 第一个储存在s0
SamplerState InputSampler : register(s0);

// 常量缓存 (b0)
cbuffer BorderImageProperties : register(b0) {
    float4  border      : packoffset(c0);
    float4  slice       : packoffset(c1);
    float4  repeat      : packoffset(c2);
    float2  size        : packoffset(c3);
    float1  calpha      : packoffset(c3.z);
};

// Shader入口
float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 sceneSpaceOutput : SCENE_POSITION,
    float4 texelSpaceInput0 : TEXCOORD0
    ) : SV_Target {
    float2 pos = texelSpaceInput0.xy;
    pos *= size;
    bool center = false;
    float4 return_adjust = float4(1, 1, 1, 1);
    // 上边
    if (pos.y < border.y) pos.y *= slice.y / border.y;
    // 下边
    else if (pos.y > border.z) pos.y = 1 - (1 - pos.y)*slice.z / (1 - border.z);
    // 中间
    else {
        center = true;
        pos.y = frac(
            (pos.y - border.y) / (border.w - border.y) * repeat.y + repeat.w
        ) * (1 - slice.w - slice.y) + slice.y;
    }

    // 左边
    if (pos.x < border.x) pos.x *= slice.x / border.x;
    // 右边
    else if (pos.x > border.z) pos.x = 1 - (1 - pos.x)*slice.w / (1 - border.w);
    // 中间
    else {
        pos.x = frac(
            (pos.x - border.x) / (border.z - border.x) * repeat.x + repeat.z
        ) * (1 - slice.z - slice.x) + slice.x;
        // 正中间
        if (center) return_adjust = float4(calpha, calpha, calpha, calpha);
    }
    return InputTexture.Sample(InputSampler, pos.xy) * return_adjust;
}
