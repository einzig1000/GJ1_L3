#ifndef COIN_FUNCTIONS_HLSLI
#define COIN_FUNCTIONS_HLSLI

struct Particle
{
    float3 translate;
    float lifeTime;
    
    float3 scale;
    float currentTime;
    
    float3 velocity;
    float rotateY;
    
    float4 color;
    
    float3 middlePos;
    float padding2;

};
struct EndPos
{
    float3 endPos;
    float easeStartTime;
};

struct EmitterForCoin
{
    float3 translate;
    uint count;
    float frequency; 
    float frequencyTime; 
    uint emit; 
    float randomRange;
    float lifeTime;
    float3 padding;
};

#endif //COIN_FUNCTIONS_HLSLI