#pragma once
#include<Game.h>
#include"../Particle/Particle.h"

class GlassParticle
{
public:
	GlassParticle();
	~GlassParticle();
	void Initialize();
	void Update(int32_t cameraID);
	void Draw();
	void SetEmitterPos(Vector3 pos);
public:

	//EmitterSphere　
	EmitterSphere emitterSphere_;

	std::vector<int32_t> modelIDs_;

	const int32_t maxParticle_ = 20;

	//グラスモデルのテクスチャ
	int32_t textureID_ = -1;
	//パーティクル
	std::vector <std::unique_ptr<RenderObject>> particles_;

	std::vector <std::unique_ptr<ComputeObject>>emitComputes_;
	std::vector <std::unique_ptr<ComputeObject>>initializeComputes_;
	std::vector <std::unique_ptr<ComputeObject>>updateComputes_;

	std::vector < int32_t>particleSRVIDs_;
	std::vector < int32_t> freeListIndexSRVIDs_;
	std::vector < int32_t> freeListSRVIDs_;
private:
	void Load(const std::string directoryName = "assets/application/model/GlassPieces/piece", const int max = 20);
};

