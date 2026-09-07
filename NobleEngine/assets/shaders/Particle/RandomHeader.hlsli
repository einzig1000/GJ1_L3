#ifndef RANDOM_FUNCTIONS_HLSLI
#define RANDOM_FUNCTIONS_HLSLI

float rand3dTo1d(float3 value)
{
    float3 smallValue = sin(value);
    float random = dot(smallValue, float3(12.9898, 78.233, 37.719));
    random = frac(sin(random) * 143758.5453);
    return random;
}

float3 rand3dTo3d(float3 value)
{
    float r0 = rand3dTo1d(value);
    float r1 = rand3dTo1d(value + float3(12.34, 56.78, 90.12));
    float r2 = rand3dTo1d(value + float3(98.76, 54.32, 10.12));
    return float3(r0, r1, r2);
}

class RandomGenerator
{
    float3 seed;
    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
};


struct Seed
{
    float3 seed;
};

#endif // RANDOM_FUNCTIONS_HLSLI