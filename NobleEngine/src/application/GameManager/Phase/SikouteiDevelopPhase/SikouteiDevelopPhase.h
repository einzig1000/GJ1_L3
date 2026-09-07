#pragma once
#include <GameManager/Phase/IPhase.h>
#include <definition/constexprs.h>
#include <Game.h>

class Glass;
class TableObject;
class Table;
class CollisionManager;
class CocktailWater;

class SikouteiDevelopPhase :
	public IPhase
{
public:
	SikouteiDevelopPhase();
	~SikouteiDevelopPhase() override;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void CheckColliders();
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }


private:
	bool LoadObstacleData(int32_t stage);
	void SaveObstacleData(int32_t stage);

	int32_t c_main_ = -1;
	float cameraTheta = 0.0f;

	//コリジョン管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	bool isDebugDraw_ = false;

	// グラス
	std::unique_ptr<Glass> glass_;
	std::unique_ptr<CocktailWater> cocktailWater_;
	Vector2 velocity_ = Vector2(0.0f, 0.0f);		// 射出速度
	Vector2 dragStartPos_ = Vector2(0.0f, 0.0f);	// マウスドラッグ開始位置

	// 障害物
	std::unique_ptr<TableObject> obstacles_[Constexprs::kMaxObstacleCount];
	int32_t obstacleCount = 0;
	int32_t deleteIndex = -1;

	// テーブル
	std::unique_ptr<Table> table_;

	// 人間
	std::unique_ptr<RenderObject> human_[3];
	EulerTransforms humanTransforms_[3];
	float humanRotate[3] = { 90.0f, 210.0f, 330.0f, };	// 人間がテーブルから見てどの角度にいるか
	int32_t currentGlassUserIndex_ = 0;					// 現在グラスを持っている人間のインデックス
	float humansize_ = 30.0f;							// キャッチ出来る角度

	// マーカー(デバッグ描画)
	std::unique_ptr<RenderObject> markers_[6];
	EulerTransforms markerTransforms_[6];
	std::vector<float> markerAngles_;					// マーカーがテーブルから見てどの角度にいるか

};


