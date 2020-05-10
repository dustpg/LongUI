// 2D纹理 第一个输入储存在t0
Texture2D InputTexture : register(t0);
// 采样器状态 第一个储存在s0
SamplerState InputSampler : register(s0);

// 常量缓存 (b0)
cbuffer Properties : register(b0) {
    float4  source      : packoffset(c0);
    float4  round       : packoffset(c1);
};

// Shader入口
float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 sceneSpaceOutput : SCENE_POSITION,
    float4 texelSpaceInput0 : TEXCOORD0
) : SV_Target{
    // 偏移半像素
    float2 offset = float2(0.5, 0.5) * texelSpaceInput0.zw;
    // round可能会拉伸图片, 所以先计算
    float2 round_pos = (texelSpaceInput0.xy - offset) * round.xy;
    // 计算采样地点
    float2 real_pos = frac(round_pos / source.zw) * source.zw;
    //float2 real_pos = round_pos % source.zw;
    // 采样
    return InputTexture.Sample(InputSampler, real_pos + source.xy + offset);
}
