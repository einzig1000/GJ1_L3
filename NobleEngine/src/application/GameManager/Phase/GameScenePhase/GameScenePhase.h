#pragma once
#include <GameManager/Phase/IPhase.h>
#include <memory>
#include <definition/constexprs.h>

class Glass;
class TableObject;
class Table;
class CollisionManager;
class CocktailWater;
class PredictionObj;
class HumanManager;//人間管理
class Bar;//バークラスの追加
class UIManager;
class GameLight;//ライト

class GameScenePhase :
	public IPhase
{
public:

	struct LayerTextureIndexData
	{
		struct { int32_t index; int32_t _pad[3]; } textures[8];
		int32_t textureCount;
		int32_t _pad[3];
	};
public:

	GameScenePhase();
	~GameScenePhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:

	std::unique_ptr<RenderObject> drawForMain_[2];
	int32_t rt_3D_ = -1;
	int32_t stageSum = 0;

	bool LoadObstacleData(int32_t stage);
	void SaveObstacleData(int32_t stage);

	int32_t s_GameScene_ = 0;
	std::vector<int32_t> s_GameScene_PlayIDs_;
	float volume = 0.0f;

	int32_t c_main_ = -1;
	Coordinate_spherical cameraSpherical_ = { 0.0f, 0.0f, 0.0f };
	CameraPhase cameraPhase_ = CameraPhase::Free;
	CounterSec cameraPhaseCounter_;
	void ChangeCameraPhase(CameraPhase phase);
	void UpdateCameraPhase();

	//コリジョン管理
	void CheckColliders();

	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	bool isDebugDraw_ = false;

	// グラス
	bool isShot_ = false;
	std::unique_ptr<Glass> glass_;
	std::unique_ptr<CocktailWater> cocktailWater_;
	Vector2 velocity_ = Vector2(0.0f, 0.0f);		// 射出速度
	Vector2 dragStartPos_ = Vector2(0.0f, 0.0f);	// マウスドラッグ開始位置
	bool ableDrag_ = true;
	float mouseInsensitivity_ = 0.020f;	// マウス感度
	bool dragging_ = false;	// ドラッグ中かどうか

	// 障害物
	std::vector<std::unique_ptr<TableObject>> obstacles_;
	int32_t deleteIndex = -1;
	// そのステージで壊せる最大数
	int32_t maxBreakableObstacleCount_ = 0;

	// テーブル
	std::unique_ptr<Table> table_ = nullptr;

	GlassType glassType = GlassType::Champagne;

	//人間管理
	std::unique_ptr<HumanManager>humanManager_ = nullptr;
	//バー
	std::unique_ptr<Bar>bar_ = nullptr;
	//ゲーム用ライト
	std::unique_ptr<GameLight> gameLight_ = nullptr;
	//予測オブジェ
	std::unique_ptr<PredictionObj>prediction_ = nullptr;
	//UI管理
	std::unique_ptr<UIManager>uiManager_ = nullptr;

};