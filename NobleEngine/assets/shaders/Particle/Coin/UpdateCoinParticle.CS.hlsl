
#include "CoinParticle.hlsli"

static const uint kMaxParticles = 1024;

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

cbuffer gPerFrame : register(b0)
{
    float deltaTime;
};
ConstantBuffer<EndPos> gEndPos : register(b1);

float F_IN_OUT_QUAD_Time(float t)
{
    float easedT = t < 0.5f ? 2.0f * t * t
			: 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    return easedT;
}

float3 EaseInOutQuad(const float3 start, const float3 end, const float time)
{
    return lerp(start, end, F_IN_OUT_QUAD_Time(time));
};

[numthreads(1024, 1, 1)]

    void main
    (
    uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    
    if (particleIndex < kMaxParticles)
    {

        // 1. すでに死亡しているパーティクルは更新を行わない（FreeListの連続加算を防止）
        if (gParticles[particleIndex].currentTime >= gParticles[particleIndex].lifeTime)
        {
            gParticles[particleIndex].lifeTime = 0.0f;
            gParticles[particleIndex].color = float4(0.0f, 0.0f, 0.0f, 0.0f);
            gParticles[particleIndex].scale = float3(0.0f, 0.0f, 0.0f);
            gParticles[particleIndex].middlePos = float3(0.0f, 0.0f, 0.0f);
            
            gParticles[particleIndex].rotateY = 0.0f;
            
            // 死んでいるparticleはfreeListに戻す
            int32_t freeListIndex;
            InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
            //最新のFreeListIndexの場所に死んだParticleのIndexを設定する。
            if ((freeListIndex + 1) < kMaxParticles)
            {
                gFreeList[freeListIndex + 1] = particleIndex;
            }
            else
            {
                //ここにくるはずはない。きたら何かが間違っているが、安全策をうっておく
                InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            }
            
            // ★ 返却完了したらこれ以上処理しないよう return する
            return;
        }
        
        gParticles[particleIndex].currentTime += deltaTime;

        if (gParticles[particleIndex].currentTime < gEndPos.easeStartTime)
        {
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
            gParticles[particleIndex].middlePos = gParticles[particleIndex].translate;
            gParticles[particleIndex].rotateY = gParticles[particleIndex].currentTime/gEndPos.easeStartTime*2.0f*3.141592f;

        }
        else if (gParticles[particleIndex].currentTime < gParticles[particleIndex].lifeTime)
        {
            gParticles[particleIndex].rotateY = 0.0f;
            float time = (gParticles[particleIndex].currentTime - gEndPos.easeStartTime) / (gParticles[particleIndex].lifeTime - gEndPos.easeStartTime );
            time = saturate(time);
            //イージングする
            gParticles[particleIndex].translate = EaseInOutQuad(gParticles[particleIndex].middlePos, gEndPos.endPos, time);
        }
    }
}
