
struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 texCoord : TEXCOORD2;
};


cbuffer WVP : register(b0)
{
    float4x4 wvp;
}

cbuffer World : register(b1)
{
    float4x4 world;
}

StructuredBuffer<Vertex> gVertices : register(t0);

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    Vertex vertex = gVertices[vertexID];
    output.position = mul(vertex.position, wvp);
    float4 worldPos = mul(vertex.position, world);
    output.worldPosition = worldPos.xyz;
    output.texCoord = vertex.texcoord;
    float3 worldNormal = mul(vertex.normal, (float3x3) world);
    output.normal = normalize(worldNormal);
    return output;
}
