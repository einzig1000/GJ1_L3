#pragma once
#include <GameManager/Phase/IPhase.h>
#include <definition/constexprs.h>
#include <Game.h>

class Glass;
class TableObject;
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
	std::unique_ptr<TableObject> obstacles_[Constexprs::kMaxObstacleCount];
	int32_t obstacleCount = 0;
	std::unique_ptr<Table> table_;

	bool LoadObstacleData(int32_t stage);
	void SaveObstacleData(int32_t stage);

	//コリジョン管理
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	bool isDebugDraw_ = false;

	Vector2 velocity_ = Vector2(0.0f, 0.0f);
	Vector2 dragStartPos_ = Vector2(0.0f, 0.0f);



	int32_t currentGlassUserIndex_ = 0;
	float cameraTheta = 0.0f;

	std::unique_ptr<RenderObject> human_[3];
	EulerTransforms humanTransforms_[3];
	float humanRotate[3] = { 90.0f, 210.0f, 330.0f, };

	std::unique_ptr<RenderObject> markers_[6];
	EulerTransforms markerTransforms_[6];
	std::vector<float> markerAngles_;
	float humansize_ = 30.0f;



	//予測オブジェ
	std::unique_ptr<PredictionObj>prediction_ = nullptr;
};


