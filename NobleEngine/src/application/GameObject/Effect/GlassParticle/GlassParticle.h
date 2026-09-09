#pragma once
#include<Game.h>
#include"../Particle/Particle.h"

class GlassParticle
{
public:

//グラス用構造体
	struct EmitterSphereForGlass
	{
		Vector3 translate; // 座標
		float radius; // 射出半径

		uint32_t count; // 射出数
		float frequency; // 射出頻度
		float frequencyTime; // 射出頻度タイマ
		uint32_t emit; // 射出するかどうか

		float speedRange;//速度範囲
		Vector3 reflectDirection;//反射方向

		Vector4 color;//開始色
	};

	struct HitPosition
	{
		Vector3 tableCenter;//テーブルセンター
		float tableRadius;//テーブル半径

		float tableThickness; //テーブルの厚み
		float floorHeight;//床の高さ
		float pieceRadius;//破片の半径
		float coefficiendOfRestituion; //反発係数
		float pieceMass;//破片の質量 
		Vector3 padding;
	};

	GlassParticle();
	~GlassParticle();
	void Initialize();
	void Update(int32_t cameraID);
	void Draw(int32_t renderTargetID);

	void DebugImGui(int32_t id);
	void SetEmitColor(const Vector4& color);
	void Emit(const Vector3& pos,const Vector3& reflectDirection= {0.0f,0.0f,0.0f});
	/// @brief 外部からテーブルの大きさと半径を得る
	/// @param center 
	/// @param radius 
	void SetTableCenterAndRadius(const Vector3& center, const float radius);
public:

	//EmitterSphereForGlass　
	EmitterSphereForGlass emitterSphere_;

	HitPosition hitPosition_;

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

