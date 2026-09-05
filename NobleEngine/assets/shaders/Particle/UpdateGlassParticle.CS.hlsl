
struct Particle
{
    float3 translate;
    float3 scale;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 color;
};

struct HitPosition
{
    float2 tableCenter;//テーブルセンター
    float tableRadius;//テーブル半径
    float tableHeight;//テーブルの高さ
    float floorHeight;//床の高さ
    float pieceRadius;//破片の半径
    float upwardForce;//上方向の力
    float padding;
};

static const uint kMaxParticles = 1024;

RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

cbuffer gPerFrame : register(b0)
{
    float deltaTime;
};

ConstantBuffer<HitPosition> gHitPosition : register(b1);

bool IsCollision(const float2  c1,const float r1, const float2 c2,const float r2)
{
      //2つの急の中心点間距離を求める 
    return distance(c1, c2) <= (r1 + r2);
}

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex < kMaxParticles)
    {
        // alphaが0のparticleは死んでいるとみなして更新しない
        if (gParticles[particleIndex].color.a != 0)
        {
            //下方向にベクトルを足す
            //gParticles[particleIndex].velocity.y -= 9.8 * deltaTime;
            
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity;   
            
            gParticles[particleIndex].currentTime += deltaTime;
            
            float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifeTime);
            
            gParticles[particleIndex].color.a = saturate(alpha);
            
            //テーブル高さより上だったとき
            if (gParticles[particleIndex].translate.y <= gHitPosition.tableHeight)
            {
                float2 particleCenterXZ = gParticles[particleIndex].translate.xz;
                //サークルとパーティクルが当たっているかを取得する。
                if (IsCollision(gHitPosition.tableCenter, gHitPosition.tableRadius, particleCenterXZ, gHitPosition.pieceRadius))
                {
                    gParticles[particleIndex].velocity.y += gHitPosition.upwardForce * deltaTime;
                }

            }  else if (gParticles[particleIndex].translate.y <= gHitPosition.floorHeight)
            {
                //床の位置だったら
                gParticles[particleIndex].velocity.y += gHitPosition.upwardForce * deltaTime;
                //上に跳ねる
            }
        
        } else if (gParticles[particleIndex].lifeTime != 0.0f)
        {
            gParticles[particleIndex].scale = float32_t3(0.0f, 0.0f, 0.0f);
            gParticles[particleIndex].lifeTime = 0.0f;
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
        }
    }
}
