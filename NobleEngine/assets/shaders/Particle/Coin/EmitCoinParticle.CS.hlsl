#include "../RandomHeader.hlsli"
#include "CoinParticle.hlsli"

static const uint kMaxParticles = 1024;

ConstantBuffer<EmitterForCoin> gEmitter : register(b0);
ConstantBuffer<Seed> gSeed : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

// 今回スレッド数は1。複数のEmitterを扱い、同時に処理したいような場合は適宜スレッド数を増やすと良い
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gEmitter.emit != 0) // 射出許可が出たので射出
    {
        RandomGenerator generator;
        generator.seed = gSeed.seed;

        for (uint countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            
            int32_t freeListIndex;
            //FreeListのIndexを1つ前に設定し、現在のIndexを取得する
            InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            if (0 <= freeListIndex && freeListIndex < kMaxParticles)
            {
                uint32_t particleIndex = gFreeList[freeListIndex];
                // カウント分Particleを射出する
                //スケールは固定値
                gParticles[particleIndex].scale = float3(0.33f, 0.33f, 0.33f);
                gParticles[particleIndex].translate = gEmitter.translate + (generator.Generate3d() * 2.0f - 1.0f) * gEmitter.randomRange;
                float3 randomDirection = generator.Generate3d();
                
                gParticles[particleIndex].velocity.x = (randomDirection.x * 2.0f - 1.0f) * 0.05f;
                gParticles[particleIndex].velocity.y = -0.03f * (1.0f - randomDirection.y*0.01f);
                gParticles[particleIndex].velocity.z = 0.0f;
                
                gParticles[particleIndex].color = float4(1.0f,1.0f,1.0f,1.0f);
                gParticles[particleIndex].lifeTime = gEmitter.lifeTime;
                gParticles[particleIndex].currentTime = 0.0f;
            }
            else
            {
                //発生させられなかったので、減らしてしまった分もとに戻す。これを忘れると発生させられなかった分だけどんどんIndexが減ってしまう
                InterlockedAdd(gFreeListIndex[0], 1);
                //Emit中にParticleは消えないので、この後発生することはないためbreakして終わらせる
                break;
            }
        }
    }
}
