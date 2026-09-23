#include "CoinParticle.hlsli"

struct VSInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

cbuffer PerView : register(b0)
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
};

StructuredBuffer<Particle> gParticles : register(t0);

VSOutput main(VSInput input, uint instanceId : SV_InstanceID)
{
    VSOutput output;

    // インスタンスIDに対応するパーティクル情報を取得
    Particle particle = gParticles[instanceId];

    
    // 1. Y軸の回転行列を作成 (particle.angle はラジアン単位の角度)
    float cosA = cos(particle.rotateY);
    float sinA = sin(particle.rotateY);

    float4x4 rotY =
    {
        cosA, 0.0f, sinA, 0.0f,
     0.0f, 1.0f, 0.0f, 0.0f,
    -sinA, 0.0f, cosA, 0.0f,
     0.0f, 0.0f, 0.0f, 1.0f
    };

// 2. ビルボード行列にY軸回転を適用
    float4x4 worldMatrix = mul(rotY, billboardMatrix);
 
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate;

    output.position = mul(input.position, mul(worldMatrix, viewProjection));
    output.texcoord = input.texcoord;
    output.color = particle.color;

    return output;
}
