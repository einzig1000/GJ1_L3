
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

cbuffer TextureIndex1 : register(b0)
{
    int textureIndex1;
};

cbuffer TextureIndex2 : register(b1)
{
    int textureIndex2;
};

cbuffer MaskTextureIndex : register(b2)
{
    int maskTextureIndex;
};

// 指定色(マスクテクスチャがこの色のピクセルはテクスチャ2を描画する)
cbuffer MaskKeyColor : register(b3)
{
    float3 keyColor;
};

// 閾値(指定色との誤差許容範囲。マスクの補間やテクスチャ圧縮で色が完全一致しない場合に対応)
cbuffer Threshold : register(b4)
{
    float threshold;
};


Texture2D<float4> textures[] : register(t0);
SamplerState gSampler : register(s0);


PSOutput main(PSInput input)
{
    PSOutput output;

    float2 uv1 = input.TexCoord;
    float2 uv2 = input.TexCoord;
    float2 uvMask = input.TexCoord;
    
    float3 maskColor = textures[maskTextureIndex].Sample(gSampler, uvMask).rgb;

    // マスクの色が指定色と一致していればテクスチャ2、それ以外はテクスチャ1
    bool isKeyColor = distance(maskColor, keyColor) <= threshold;
    if (isKeyColor)
    {
        output.Color = textures[textureIndex2].Sample(gSampler, uv2);
    }
    else
    {
        output.Color = textures[textureIndex1].Sample(gSampler, uv1);
    }

    return output;
}
