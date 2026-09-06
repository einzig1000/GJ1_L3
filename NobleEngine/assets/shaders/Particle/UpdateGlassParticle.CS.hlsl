
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
    float3 tableCenter; //テーブルセンター
    float tableRadius; //テーブル半径
    
    float tableThickness; //テーブルの厚み
    float floorHeight; //床の高さ
    float pieceRadius; //破片の半径
    float coefficiendOfRestituion; //反発係数
    
    float pieceMass; //破片の質量 
    float3 padding;
};

static const uint kMaxParticles = 1024;
static const float gravity = 9.8;
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);

cbuffer gPerFrame : register(b0)
{
    float deltaTime;
};

ConstantBuffer<HitPosition> gHitPosition : register(b1);

bool IsCollision(const float2 c1, const float r1, const float2 c2, const float r2)
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
      
            gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
            gParticles[particleIndex].currentTime += deltaTime;
            float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifeTime);
            gParticles[particleIndex].color.a = saturate(alpha);
            
            //下方向にベクトルを足す
            gParticles[particleIndex].velocity.y -= gravity * deltaTime * 0.01;
            
            bool isReflect = false;
            
            if (gParticles[particleIndex].translate.y <= gHitPosition.floorHeight)
            {
                //床の位置だったら
                isReflect = true;
                //上に跳ねる
            }
            else if (IsCollision(gHitPosition.tableCenter.xz, gHitPosition.tableRadius, gParticles[particleIndex].translate.xz, gHitPosition.pieceRadius))
            {
                  //サークルとパーティクルが当たっているかを取得する。
                
                float tableHalfThickness = gHitPosition.tableThickness * 0.5f;
                float tableHeight = gHitPosition.tableCenter.y;
                //基準値からの幅の範囲の入っているか判定する
                if (gParticles[particleIndex].translate.y >= tableHeight - tableHalfThickness &&
                
                gParticles[particleIndex].translate.y <= tableHeight + tableHalfThickness)
                {
                    //上に跳ねる
                    isReflect = true;
                }

            }
       
            if (isReflect)
            {
  
                gParticles[particleIndex].velocity.y = -gHitPosition.coefficiendOfRestituion * gParticles[particleIndex].velocity.y;
                
                const float3 absVel = abs(gParticles[particleIndex].velocity);
                //摩擦係数
              
                if (absVel.x > 0.0f || absVel.y > 0.0f || absVel.z > 0.0f)
                {
                    //絶対値が0以上の時
                    float friction = 0.8f;
                    //反対方向に対して摩擦力をかける
                    float2 frictionVel = -gParticles[particleIndex].velocity.xz * friction * gravity * gHitPosition.pieceMass * 0.01;
                    
                    gParticles[particleIndex].velocity.xz += frictionVel;
                }
                
            }
        
        }
        else if (gParticles[particleIndex].lifeTime != 0.0f)
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
