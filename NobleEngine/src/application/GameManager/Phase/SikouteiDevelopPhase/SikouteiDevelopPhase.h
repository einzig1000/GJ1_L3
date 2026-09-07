#pragma once
#include <GameManager/Phase/IPhase.h>
#include <Game.h>

class Glass;
class Obstacle;
class Table;
class CollisionManager;
class PredictionObj;

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
	int32_t c_main_ = -1;

	//ゲームオブジェクト
	int32_t deleteIndex = -1;
	std::unique_ptr<Glass> glass_;
	std::vector<std::unique_ptr<Obstacle>> obstacles_;
	std::unique_ptr<Table> table_;

	bool LoadObstacleData(int32_t stage);
	void SaveObstacleData(int32_t stage);

	//コリジョン管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	bool DisebugDraw_ = false;

	Vector2 velocity_ = Vector2(0.0f, 0.0f);
	Vector2 dragStartPos_ = Vector2(0.0f, 0.0f);

	std::unique_ptr<PredictionObj>predictionObj_ = nullptr;
};

