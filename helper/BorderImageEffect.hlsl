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


/*
| D0     | D1     |D2    |

--------------------------
| A      | B      | C    |
--------------------------
|      /           \     |
|     /             \    |
--------------------------
|X  |Y               |Z  |
--------------------------

|S0 |        S1      |S2 |

if (x in 0)  f(x) = x * D0 / S0
if (x in 1)  f(x) = (x - S0) * D1 / S1 + D1
if (x in 2)  f(x) = 1 - (1-x) * D2 / S2

*/

float fx0(float x) {
    return x * slice.x / border.x;
}

float fx1(float x) {
    const float offset = (repeat.x - floor(repeat.x)) * 0.5;
    return frac((x - border.x) / (border.z - border.x)*repeat.x - offset)*(1 - slice.z - slice.x) + slice.x;
}

float fx2(float x) {
    return 1 - (1 - x) * slice.z / (1 - border.z);
}

float fx3(float x) {
    const float offset = (repeat.z - floor(repeat.z)) * 0.5;
    return frac((x - border.x) / (border.z - border.x)*repeat.z - offset)*(1 - slice.z - slice.x) + slice.x;
}



float fy0(float y) {
    return y * slice.y / border.y;
}

float fy1(float y) {
    const float offset = (repeat.y - floor(repeat.y)) * 0.5;
    return frac((y - border.y) / (border.w - border.y)*repeat.y - offset)*(1 - slice.w - slice.y) + slice.y;
}

float fy2(float y) {
    return 1 - (1 - y)*slice.w / (1 - border.w);
}

float fy3(float y) {
    const float offset = (repeat.w - floor(repeat.w)) * 0.5;
    return frac((y - border.y) / (border.w - border.y)*repeat.w - offset)*(1 - slice.w - slice.y) + slice.y;
}

// Shader入口
float4 main(
    float4 clipSpaceOutput  : SV_POSITION,
    float4 sceneSpaceOutput : SCENE_POSITION,
    float4 texelSpaceInput0 : TEXCOORD0
) : SV_Target{
#if 0
    const float2 pos = texelSpaceInput0.xy / (size * texelSpaceInput0.zw);
float3 fx = float3(fx0(pos.x), fx1(pos.x), fx2(pos.x));
const float3 if0 = float3(
    step(pos.x, border.x),
    step(border.x, pos.x) * step(pos.x, border.z),
    step(border.z, pos.x)
    );
float3 fy = float3(fy0(pos.y), fy1(pos.y), fy2(pos.y));

const float3 if1 = float3(
    step(pos.y, border.y),
    step(border.y, pos.y) * step(pos.y, border.w),
    step(border.w, pos.y)
    );

fx *= if0;
fy *= if1;

const float alpha = 1 - if0.y * if1.y * (1 - calpha);

return InputTexture.Sample(InputSampler, float2(fx.x + fx.y + fx.z, fy.x + fy.y + fy.z))
* float4(alpha, alpha, alpha, alpha);
#else
    const float2 now = texelSpaceInput0.xy / (size * texelSpaceInput0.zw);
float2 pos;
bool center = false;
float4 return_adjust = float4(1, 1, 1, 1);
// 上边
if (now.y < border.y) pos.y = fy0(now.y);
// 下边
else if (now.y > border.w) pos.y = fy2(now.y);
// 中间
else {
    center = true;
    pos.y = (now.x > border.z) ? fy3(now.y) : fy1(now.y);
}

// 左边
if (now.x < border.x) pos.x = fx0(now.x);
// 右边
else if (now.x > border.z) pos.x = fx2(now.x);
// 中间
else {
    pos.x = (now.y > border.w) ? fx3(now.x) : fx1(now.x);
    // 正中间
    if (center) return_adjust = float4(calpha, calpha, calpha, calpha);
}
return InputTexture.Sample(InputSampler, pos.xy) * return_adjust;
#endif
}
