// SimpleModel.VS.hlsl

struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;

    // DirectXから渡されるインスタンス番号
    uint instanceID : SV_InstanceID;
};


struct VSOutput
{
    float4 position : SV_POSITION;

    float2 texCoord : TEXCOORD0;

    float3 normal : NORMAL0;

    float3 worldPosition : TEXCOORD1;

    // PSへそのまま渡す
    nointerpolation uint instanceID : TEXCOORD2;
};


cbuffer ViewProjectionBuffer : register(b0)
{
    float4x4 viewProjection;
};


cbuffer WorldMatrixBuffer : register(b1)
{
    uint worldMatrixHeapSlot;
};


VSOutput main(VSInput input)
{
    VSOutput output;


    StructuredBuffer<float4x4> worldMatrixBuffer =
        ResourceDescriptorHeap[
            worldMatrixHeapSlot
        ];


    float4x4 world =
        worldMatrixBuffer[
            input.instanceID
        ];


    float4 worldPosition =
        mul(
            input.position,
            world
        );


    output.position =
        mul(
            worldPosition,
            viewProjection
        );


    output.texCoord =
        input.texCoord;


    output.worldPosition =
        worldPosition.xyz;


    float3 worldNormal =
        mul(
            input.normal,
            (float3x3) world
        );


    output.normal =
        normalize(
            worldNormal
        );


    output.instanceID =
        input.instanceID;


    return output;
}