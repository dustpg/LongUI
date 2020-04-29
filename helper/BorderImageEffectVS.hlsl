cbuffer CustomVSCBuffer : register(b1)
{
    float4 zone0    : packoffset(c0);
    float4 zone1    : packoffset(c1);
    float4 zone2    : packoffset(c2);
    float4 zone3    : packoffset(c3);
    float4 repeat   : packoffset(c4);
};

cbuffer Direct2DTransforms : register(b0)
{
    float2x1 sceneToOutputX;
    float2x1 sceneToOutputY;
    float2x1 sceneToInput0X;
    float2x1 sceneToInput0Y;
};

struct VSOut
{
    float4 clipSpaceOutput  : SV_POSITION;
    float4 texcoordData : DATA_TEXCOORD;
    float4 texelSpaceInput0 : TEXCOORD0;
};


VSOut main(
    float4 pos0 : POSITION0, 
    float4 pos1 : POSITION1,
    uint2 index : INDEX
) {
    VSOut output;
    const float4 posDes = float4(pos0.xy * zone0.xy
        + pos0.zw * zone1.xy
        + pos1.xy * zone2.xy
        , float2(0, 1));

    output.clipSpaceOutput.x = (posDes.x * sceneToOutputX[0]) + posDes.w * sceneToOutputX[1];
    output.clipSpaceOutput.y = (posDes.y * sceneToOutputY[0]) + posDes.w * sceneToOutputY[1];
    output.clipSpaceOutput.z = posDes.z;
    output.clipSpaceOutput.w = posDes.w;

    // 顶点很少(50个左右), 可以使用分支判断, 懒得修改顶点源数据
    output.texelSpaceInput0.xy = pos1.zw;
    output.texelSpaceInput0.z = sceneToInput0X[0];
    output.texelSpaceInput0.w = sceneToInput0Y[0];

    // X0
    if (index.x == 0) {
        output.texcoordData.x = zone0.z;
        output.texcoordData.z = zone1.z;
    }
    // X1
    else if (index.x == 1) {
        output.texcoordData.x = zone1.z;
        output.texcoordData.z = zone2.z;
        const float repeat_x1 = (index.y == 2) ? repeat.z : repeat.x;
        output.texelSpaceInput0.x 
            = output.texelSpaceInput0.x * repeat_x1
            + 0.5 - (repeat_x1* 0.5 - floor(repeat_x1* 0.5))
            ;
    }
    // X2
    else /*if (index.x == 2)*/ {
        output.texcoordData.x = zone2.z;
        output.texcoordData.z = zone3.z;
    }

    // Y0
    if (index.y == 0) {
        output.texcoordData.y = zone0.w;
        output.texcoordData.w = zone1.w;
    }
    // Y1
    else if (index.y == 1) {
        output.texcoordData.y = zone1.w;
        output.texcoordData.w = zone2.w;
        const float repeat_y1 = (index.x == 2) ? repeat.w : repeat.y;
        output.texelSpaceInput0.y
            = output.texelSpaceInput0.y * repeat_y1
            + 0.5 - (repeat_y1* 0.5 - floor(repeat_y1* 0.5))
            ;
    }
    // Y2
    else /*if (index.y == 2)*/ {
        output.texcoordData.y = zone2.w;
        output.texcoordData.w = zone3.w;
    }

    return output;
}