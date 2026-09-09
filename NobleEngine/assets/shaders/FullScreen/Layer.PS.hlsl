
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

static const uint kMaxLayerCount = 8;

cbuffer LayerTextureIndex : register(b0)
{
    int textureIndices[kMaxLayerCount]; // 送られてきた順(下から積む順)のテクスチャID
    int textureCount; // 実際に送られてきたテクスチャの枚数
};

Texture2D<float4> textures[] : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    PSOutput output;

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    uint count = min((uint) textureCount, kMaxLayerCount);
    for (uint i = 0; i < count; ++i)
    {
        float4 layerColor = textures[textureIndices[i]].Sample(gSampler, input.TexCoord);

        // 後から送られたテクスチャほど上に重なるようにOver合成する
        color.rgb = layerColor.rgb * layerColor.a + color.rgb * (1.0f - layerColor.a);
        color.a = layerColor.a + color.a * (1.0f - layerColor.a);
    }

    output.Color = color;
    return output;
}