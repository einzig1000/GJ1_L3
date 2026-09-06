#include "RandomHeader.hlsli"

static const uint kMaxParticles = 1024;

//グラス用構造体
struct EmitterSphereForGlass
{
    float3 translate; // 座標
    float radius; // 射出半径
    
    uint count; // 射出数
    float frequency; // 射出頻度
    float frequencyTime; // 射出頻度タイマ
    uint emit; // 射出するかどうか
   
    float speedRange;//速度範囲
    float3 reflectDirection;//反射方向
};

struct Particle
{
    float3 translate;
    float3 scale;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 color;
};

ConstantBuffer<EmitterSphereForGlass> gEmitter : register(b0);
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
                gParticles[particleIndex].scale = (1.0f,1.0f,1.0f);
        
                gParticles[particleIndex].translate = gEmitter.translate + (generator.Generate3d() * 2.0f - 1.0f) * gEmitter.radius;
                float3 randomDirection = generator.Generate3d();
                randomDirection = randomDirection * gEmitter.speedRange - (gEmitter.speedRange*0.5);
                
                gParticles[particleIndex].velocity = randomDirection * 0.01f;
                
                //ベクトルを足す
                //gParticles[particleIndex].velocity += gEmitter.reflectDirection;

                
                gParticles[particleIndex].color.rgb = (1.0f,1.0f,1.0f);
                gParticles[particleIndex].color.a = 1.0f;
                gParticles[particleIndex].lifeTime = 1.0f;
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
