
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

// サンプラーのレジスタ番号を s0 に設定
SamplerState gSampler : register(s0);
cbuffer Texture : register(b0)
{
    int textureIndex;
};


PSOutput main(PSInput input)
{
    // UV同次座標
    PSOutput output;
    Texture2D<float4> texture = ResourceDescriptorHeap[textureIndex];
    float4 textureColor = texture.Sample(gSampler, input.texcoord);
    output.color = input.color * textureColor;
    return output;
}