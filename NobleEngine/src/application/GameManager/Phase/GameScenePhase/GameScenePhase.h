#pragma once
#include <GameManager/Phase/IPhase.h>
#include <memory>
#include <definition/constexprs.h>

class GlassManager;//グラス総括管理
class TableObject;
class Table;
class CollisionManager;
class PredictionObj;//予測オブジェ
class HumanManager;//人間管理
class Bar;//バークラスの追加
class UIManager;
class GameLight;//ライト
class GameCameraManager;//ゲームカメラ管理
class GameScreen;//ゲーム画面

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
	void UpdateBGM();
	void ResetGame();
	
	void DrawMainScreen(const int32_t renderTexture);
	//データ保存
	bool LoadObstacleData(int32_t stage);
	void SaveObstacleData(int32_t stage);
	//プレイヤー操作
	void PlayerControl();
	//コリジョン管理
	void CheckColliders();
	//テーブル内イベント
	void InnerTableEvent();

	//ゲーム画面　
	std::unique_ptr<GameScreen>gameScreen_ = nullptr;

	int32_t stageSum = 0;

	std::unique_ptr<GameCameraManager> gameCameraManager_ = nullptr;

	std::unique_ptr<GlassManager>glassManager_ = nullptr;

	GlassType glassType = GlassType::Champagne;
	// 障害物
	std::vector<std::unique_ptr<TableObject>> obstacles_;
	int32_t deleteIndex = -1;
	// そのステージで壊せる最大数
	int32_t maxBreakableObstacleCount_ = 0;

	// テーブル
	std::unique_ptr<Table> table_ = nullptr;
	//人間管理
	std::unique_ptr<HumanManager>humanManager_ = nullptr;
	//バー
	std::unique_ptr<Bar>bar_ = nullptr;
	//ゲーム用ライト
	std::unique_ptr<GameLight> gameLight_ = nullptr;
	//UI管理
	std::unique_ptr<UIManager>uiManager_ = nullptr;

	bool isDebugDraw_ = false;
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;


	bool canControll_ = true;
};