#pragma once
#include<Game.h>
#include"../Particle/Particle.h"

class CoinParticle
{
public:

	struct CoinParticleData
	{
		Vector3 translate;
		float lifeTime;

		Vector3 scale;
		float currentTime;

		Vector3 velocity;
		float padding1;

		Vector4 color;

		Vector3 middlePos;
		float padding2;
	};

    struct EmitterForCoin {
		Vector3 translate; // 座標
		uint32_t count; // 射出数
		float frequency; // 射出頻度
		float frequencyTime; // 射出頻度タイマ
		uint32_t emit; // 射出するかどうか
		float randomRange;//射出範囲
		float lifeTime;
		Vector3 padding;
    };

	struct EndPos
	{
		Vector3 endPos;
		float easeStartTime;
	};

public:
	CoinParticle();
	~CoinParticle();

	void Initialize();
	void Update(int32_t cameraID);
	void Draw(int32_t renderTargetID);
	void SetPosition(const Vector3& startPos,const Vector3& endPos);
	void Emit(const int32_t emitCount);
	void DebugImGui();

private:
	EmitterForCoin emitterForCoin_;
	EndPos endpos_;
	int32_t modelID_ = -1;
	int32_t textureID_ = -1;

	std::unique_ptr<RenderObject> particle_;

	std::unique_ptr<ComputeObject> emitCompute_;
	std::unique_ptr<ComputeObject> initializeCompute_;
	std::unique_ptr<ComputeObject> updateCompute_;

	int32_t particleSRVID_;
	int32_t freeListIndexSRVID_;
	int32_t freeListSRVID_;


};

