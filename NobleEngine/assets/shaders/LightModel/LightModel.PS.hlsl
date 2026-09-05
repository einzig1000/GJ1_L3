// LightModel.PS.hlsl

#define MAX_LIGHT_COUNT 20

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2
#define LIGHT_TYPE_AREA        3


struct Light
{
    int type;
    float3 padding0;

    float4 color;

    float intensity;
    float3 padding1;

    float3 direction;
    float padding2;

    float3 position;
    float radius;

    float decay;
    float distance;
    float cosAngle;
    float cosFalloffStart;
};


cbuffer TextureBufferData : register(b0)
{
    uint colorHeapSlot;
    uint textureIndexHeapSlot;
};


cbuffer LightCB : register(b1)
{
    Light gLights[MAX_LIGHT_COUNT];

    int gLightCount;
    float3 gLightCountPadding;

    float3 gAmbientColor;
    float gAmbientColorPadding;
};


struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;

    // ワールド空間の法線
    float3 normal : NORMAL0;

    // ワールド空間の座標
    float3 worldPosition : TEXCOORD1;

    // 描画中のインスタンス番号
    nointerpolation uint instanceID : TEXCOORD2;
};


struct PSOutput
{
    float4 color : SV_TARGET;
};


SamplerState gSampler : register(s0);


// ========================================
// Half Lambert
// ========================================

float CalculateHalfLambert(
    float3 normal,
    float3 directionToLight)
{
    float halfLambert =
        dot(
            normal,
            directionToLight
        ) *
        0.5f +
        0.5f;

    halfLambert =
        saturate(
            halfLambert
        );

    // 暗部が明るくなりすぎないよう二乗する
    halfLambert *=
        halfLambert;

    return halfLambert;
}


// ========================================
// Distance Attenuation
// ========================================

float CalculateDistanceAttenuation(
    float lightDistance,
    float radius,
    float maxDistance,
    float decay)
{
    if (maxDistance > 0.0f &&
        lightDistance >= maxDistance)
    {
        return 0.0f;
    }

    float safeRadius =
        max(
            radius,
            0.0001f
        );

    float safeDecay =
        max(
            decay,
            0.0001f
        );

    float normalizedDistance =
        lightDistance /
        safeRadius;

    float attenuation =
        1.0f /
        (
            1.0f +
            pow(
                normalizedDistance,
                safeDecay
            )
        );

    if (maxDistance > 0.0f)
    {
        float rangeAttenuation =
            saturate(
                1.0f -
                lightDistance /
                maxDistance
            );

        rangeAttenuation *=
            rangeAttenuation;

        attenuation *=
            rangeAttenuation;
    }

    return attenuation;
}


// ========================================
// Directional Light
// ========================================

float3 CalculateDirectionalLight(
    Light light,
    float3 normal)
{
    // directionは光が進む方向なので反転
    float3 directionToLight =
        normalize(
            -light.direction
        );

    float diffuse =
        CalculateHalfLambert(
            normal,
            directionToLight
        );

    return
        light.color.rgb *
        light.intensity *
        diffuse;
}


// ========================================
// Point Light
// ========================================

float3 CalculatePointLight(
    Light light,
    float3 worldPosition,
    float3 normal)
{
    float3 toLight =
        light.position -
        worldPosition;

    float lightDistance =
        length(
            toLight
        );

    if (lightDistance <= 0.0001f)
    {
        return
            light.color.rgb *
            light.intensity;
    }

    float3 directionToLight =
        toLight /
        lightDistance;

    float diffuse =
        CalculateHalfLambert(
            normal,
            directionToLight
        );

    float attenuation =
        CalculateDistanceAttenuation(
            lightDistance,
            light.radius,
            light.distance,
            light.decay
        );

    return
        light.color.rgb *
        light.intensity *
        diffuse *
        attenuation;
}


// ========================================
// Spot Light
// ========================================

float3 CalculateSpotLight(
    Light light,
    float3 worldPosition,
    float3 normal)
{
    float3 toLight =
        light.position -
        worldPosition;

    float lightDistance =
        length(
            toLight
        );

    if (lightDistance <= 0.0001f)
    {
        return
            light.color.rgb *
            light.intensity;
    }

    // ピクセルからライトへ向かう方向
    float3 directionToLight =
        toLight /
        lightDistance;

    float diffuse =
        CalculateHalfLambert(
            normal,
            directionToLight
        );

    float distanceAttenuation =
        CalculateDistanceAttenuation(
            lightDistance,
            light.radius,
            light.distance,
            light.decay
        );

    // ライトからピクセルへ向かう方向
    float3 directionFromLight =
        -directionToLight;

    float3 spotDirection =
        normalize(
            light.direction
        );

    float cosTheta =
        dot(
            directionFromLight,
            spotDirection
        );

    // cosFalloffStartが内側
    // cosAngleが外側
    float angleRange =
        max(
            light.cosFalloffStart -
            light.cosAngle,
            0.0001f
        );

    float angleAttenuation =
        saturate(
            (
                cosTheta -
                light.cosAngle
            ) /
            angleRange
        );

    // SmoothStep補間
    angleAttenuation =
        angleAttenuation *
        angleAttenuation *
        (
            3.0f -
            2.0f *
            angleAttenuation
        );

    return
        light.color.rgb *
        light.intensity *
        diffuse *
        distanceAttenuation *
        angleAttenuation;
}


// ========================================
// Area Light
// ========================================

float3 CalculateAreaLight(
    Light light,
    float3 worldPosition,
    float3 normal)
{
    float3 toLight =
        light.position -
        worldPosition;

    float lightDistance =
        length(
            toLight
        );

    if (lightDistance <= 0.0001f)
    {
        return
            light.color.rgb *
            light.intensity;
    }

    float3 directionToLight =
        toLight /
        lightDistance;

    float diffuse =
        CalculateHalfLambert(
            normal,
            directionToLight
        );

    float attenuation =
        CalculateDistanceAttenuation(
            lightDistance,
            light.radius,
            light.distance,
            light.decay
        );

    float softness =
        saturate(
            light.radius /
            max(
                lightDistance,
                0.0001f
            )
        );

    float softDiffuse =
        lerp(
            diffuse,
            1.0f,
            softness * 0.5f
        );

    return
        light.color.rgb *
        light.intensity *
        softDiffuse *
        attenuation;
}


// ========================================
// Main
// ========================================

PSOutput main(PSInput input)
{
    PSOutput output;


    // インスタンスごとのカラー
    StructuredBuffer<float4> colorBuffer =
        ResourceDescriptorHeap[
            colorHeapSlot
        ];


    // インスタンスごとのテクスチャ番号
    StructuredBuffer<int> textureIndexBuffer =
        ResourceDescriptorHeap[
            textureIndexHeapSlot
        ];


    float4 modelColor =
        colorBuffer[
            input.instanceID
        ];


    int textureIndex =
        textureIndexBuffer[
            input.instanceID
        ];


    Texture2D<float4> targetTexture =
        ResourceDescriptorHeap[
            textureIndex
        ];


    float4 texColor =
        targetTexture.Sample(
            gSampler,
            input.texCoord
        );


    float4 baseColor =
        modelColor *
        texColor;


    if (baseColor.a < 0.1f)
    {
        discard;
    }


    float3 normal =
        normalize(
            input.normal
        );


    float3 lighting =
        gAmbientColor;


    int lightCount =
        min(
            max(
                gLightCount,
                0
            ),
            MAX_LIGHT_COUNT
        );


    for (int index = 0;
         index < lightCount;
         ++index)
    {
        Light light =
            gLights[index];


        if (light.type ==
            LIGHT_TYPE_DIRECTIONAL)
        {
            lighting +=
                CalculateDirectionalLight(
                    light,
                    normal
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_POINT)
        {
            lighting +=
                CalculatePointLight(
                    light,
                    input.worldPosition,
                    normal
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_SPOT)
        {
            lighting +=
                CalculateSpotLight(
                    light,
                    input.worldPosition,
                    normal
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_AREA)
        {
            lighting +=
                CalculateAreaLight(
                    light,
                    input.worldPosition,
                    normal
                );
        }
    }


    output.color.rgb =
        baseColor.rgb *
        lighting;


    output.color.a =
        baseColor.a;


    return output;
}