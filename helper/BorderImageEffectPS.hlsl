Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 texcoordData     : DATA_TEXCOORD,
    float4 texelSpaceInput0 : TEXCOORD0
) : SV_Target {
    const float2 nor = frac(texelSpaceInput0.xy);
    const float2 pos = lerp(texcoordData.xy, texcoordData.zw, nor);
    return InputTexture.Sample(InputSampler, pos);
}