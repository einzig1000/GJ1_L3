// SimpleModels.PS.hlsl
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    uint instanceID : SV_InstanceID;
};

struct PSOutput
{
    float4 color : SV_TARGET;
};

cbuffer HeapSlot : register(b0)
{
    uint color;
    uint textureIndex;
};

Texture2D<float4> textures[] : register(t0);

SamplerState gSampler : register(s0);

PSOutput main(PSInput input)
{
    StructuredBuffer<float4> ColorList = ResourceDescriptorHeap[color];
    StructuredBuffer<uint> TextureIndexList = ResourceDescriptorHeap[textureIndex];
    
    PSOutput output;
    float4 texColor = textures[TextureIndexList[input.instanceID]].Sample(gSampler, input.texCoord);
    output.color = ColorList[input.instanceID] * texColor;
    if (output.color.a < 0.1f)
    {
        discard;
    }
    return output;
}

