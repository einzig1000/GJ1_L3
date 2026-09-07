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


// フォン反射で使用するカメラのワールド座標
cbuffer CameraCB : register(b2)
{
    float3 gCameraPosition;
    float gCameraPositionPadding;
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


struct LightingResult
{
    float3 diffuse;
    float3 specular;
};


SamplerState gSampler : register(s0);


// ========================================
// Smooth Diffuse Reflection
// ========================================

float CalculateSmoothDiffuse(
    float3 normal,
    float3 directionToLight)
{
    float normalDotLight =
        dot(
            normal,
            directionToLight
        );

    // 明暗の境界を広げて、モデル表面の陰影を滑らかにつなぐ
    static const float shadowStart = -0.10f;
    static const float shadowEnd = 0.50f;

    return
        smoothstep(
            shadowStart,
            shadowEnd,
            normalDotLight
        );
}


// ========================================
// Phong Reflection
// ========================================

float CalculatePhongSpecular(
    float3 normal,
    float3 directionToLight,
    float3 directionToView)
{
    float3 reflectionDirection =
        reflect(
            -directionToLight,
            normal
        );

    float reflectionAmount =
        saturate(
            dot(
                reflectionDirection,
                directionToView
            )
        );

    // ライトと反対側の暗部には鏡面反射を出さない
    float frontLightAmount =
        smoothstep(
            0.0f,
            0.1f,
            dot(
                normal,
                directionToLight
            )
        );

    // 大きいほどハイライトが小さく鋭くなる
    static const float specularPower = 32.0f;

    // 鏡面反射の強さ
    static const float specularStrength = 0.5f;

    return
        pow(
            reflectionAmount,
            specularPower
        ) *
        specularStrength *
        frontLightAmount;
}


float2 CalculatePhongLighting(
    float3 normal,
    float3 directionToLight,
    float3 directionToView)
{
    float diffuse =
        CalculateSmoothDiffuse(
            normal,
            directionToLight
        );

    float specular =
        CalculatePhongSpecular(
            normal,
            directionToLight,
            directionToView
        );

    return float2(diffuse, specular);
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

LightingResult CalculateDirectionalLight(
    Light light,
    float3 normal,
    float3 directionToView)
{
    // directionは光が進む方向なので反転
    float3 directionToLight =
        normalize(
            -light.direction
        );

    float2 phongLighting =
        CalculatePhongLighting(
            normal,
            directionToLight,
            directionToView
        );

    LightingResult result;
    float3 lightAmount =
        light.color.rgb *
        light.intensity;

    result.diffuse = lightAmount * phongLighting.x;
    result.specular = lightAmount * phongLighting.y;

    return result;
}


// ========================================
// Point Light
// ========================================

LightingResult CalculatePointLight(
    Light light,
    float3 worldPosition,
    float3 normal,
    float3 directionToView)
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
        LightingResult result;
        result.diffuse = light.color.rgb * light.intensity;
        result.specular = float3(0.0f, 0.0f, 0.0f);
        return result;
    }

    float3 directionToLight =
        toLight /
        lightDistance;

    float2 phongLighting =
        CalculatePhongLighting(
            normal,
            directionToLight,
            directionToView
        );

    float attenuation =
        CalculateDistanceAttenuation(
            lightDistance,
            light.radius,
            light.distance,
            light.decay
        );

    LightingResult result;
    float3 lightAmount =
        light.color.rgb *
        light.intensity *
        attenuation;

    result.diffuse = lightAmount * phongLighting.x;
    result.specular = lightAmount * phongLighting.y;

    return result;
}


// ========================================
// Spot Light
// ========================================

LightingResult CalculateSpotLight(
    Light light,
    float3 worldPosition,
    float3 normal,
    float3 directionToView)
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
        LightingResult result;
        result.diffuse = light.color.rgb * light.intensity;
        result.specular = float3(0.0f, 0.0f, 0.0f);
        return result;
    }

    // ピクセルからライトへ向かう方向
    float3 directionToLight =
        toLight /
        lightDistance;

    float2 phongLighting =
        CalculatePhongLighting(
            normal,
            directionToLight,
            directionToView
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

    LightingResult result;
    float3 lightAmount =
        light.color.rgb *
        light.intensity *
        distanceAttenuation *
        angleAttenuation;

    result.diffuse = lightAmount * phongLighting.x;
    result.specular = lightAmount * phongLighting.y;

    return result;
}


// ========================================
// Area Light
// ========================================

LightingResult CalculateAreaLight(
    Light light,
    float3 worldPosition,
    float3 normal,
    float3 directionToView)
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
        LightingResult result;
        result.diffuse = light.color.rgb * light.intensity;
        result.specular = float3(0.0f, 0.0f, 0.0f);
        return result;
    }

    float3 directionToLight =
        toLight /
        lightDistance;

    float2 phongLighting =
        CalculatePhongLighting(
            normal,
            directionToLight,
            directionToView
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
            phongLighting.x,
            1.0f,
            softness * 0.5f
        );

    LightingResult result;
    float3 lightAmount =
        light.color.rgb *
        light.intensity *
        attenuation;

    result.diffuse = lightAmount * softDiffuse;
    result.specular =
        lightAmount *
        phongLighting.y *
        (1.0f - softness * 0.5f);

    return result;
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


    float3 directionToView =
        normalize(
            gCameraPosition -
            input.worldPosition
        );


    // 環境光と拡散反射はモデル・テクスチャ色へ掛ける
    float3 diffuseLighting =
        gAmbientColor;


    // 鏡面反射は表面色とは分け、ライト色として加算する
    float3 specularLighting =
        float3(0.0f, 0.0f, 0.0f);


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


        LightingResult lightResult;
        lightResult.diffuse = float3(0.0f, 0.0f, 0.0f);
        lightResult.specular = float3(0.0f, 0.0f, 0.0f);


        if (light.type ==
            LIGHT_TYPE_DIRECTIONAL)
        {
            lightResult =
                CalculateDirectionalLight(
                    light,
                    normal,
                    directionToView
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_POINT)
        {
            lightResult =
                CalculatePointLight(
                    light,
                    input.worldPosition,
                    normal,
                    directionToView
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_SPOT)
        {
            lightResult =
                CalculateSpotLight(
                    light,
                    input.worldPosition,
                    normal,
                    directionToView
                );
        }
        else if (light.type ==
                 LIGHT_TYPE_AREA)
        {
            lightResult =
                CalculateAreaLight(
                    light,
                    input.worldPosition,
                    normal,
                    directionToView
                );
        }


        diffuseLighting += lightResult.diffuse;
        specularLighting += lightResult.specular;
    }


    output.color.rgb =
        baseColor.rgb *
        diffuseLighting +
        specularLighting;


    output.color.a =
        baseColor.a;


    return output;
}
