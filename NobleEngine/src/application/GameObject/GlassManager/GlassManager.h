#pragma once
#include<Game.h>
class Glass;
class CocktailWater;
class Collider;
class PredictionObj;
class GameCameraManager;
class TableObject;
class CollisionManager;
class GlassManager
{
public:
    GlassManager();
    ~GlassManager();

	void Initialize();
	void Update(const int32_t cameraID);
	//ゲームカメラとの相互作用
	void PlayerControl(GameCameraManager* gameCameraManager, std::vector<std::unique_ptr<TableObject>>& tebleObjects);
	
	void DrawPrediction(const int32_t renderTexture);
	void Draw(const int32_t renderTexture);


	void DrawImGui();
	/// @brief テーブル位置と人の角度から座標をセットする関数
	/// @param tablePos テーブル位置
	/// @param tableRadius テーブル半径
	/// @param humanDeg 人の配置角度
	void SetPosForTableAndHuman(const Vector3& tablePos, const float tableRadius, const float humanDeg, const float tabelHeight = 1.28f);
	/// @brief グラスのXZ軸においての位置を取得する
	/// @return xZの位置
	Vector2 GetPos2D();

	float GetRadius();
	void AddTranslate(const Vector3& vel);
	bool IsBroken();
	std::vector<std::unique_ptr<Collider>>& GetColliders();

	const Vector3& GetTranslate();
	const float GetMouseInsensitivity();
	void SetVelocity(const Vector3& vel);
	const Vector3& GetVelocity();

	bool& GetAbleDragAddress() { return ableDrag_; }

	void SetLightData(LightDataForGPU* data);
	void SetCollisionManager(CollisionManager* collisionManager);
	void SetIsShot(const bool isShot) { isShot_ = isShot; };
	bool& IsShotAddress() { return isShot_; }
private:

	//グラス
	std::unique_ptr<Glass> glass_ = nullptr;
	//カクテル液体
	std::unique_ptr<CocktailWater> cocktailWater_ = nullptr;
	//予測オブジェ
	std::unique_ptr<PredictionObj>prediction_ = nullptr;
	// 射出速度
	Vector2 velocity_ = Vector2(0.0f, 0.0f);
	// マウスドラッグ開始位置
	Vector2 dragStartPos_ = Vector2(0.0f, 0.0f);
	// グラスをショットしたか
	bool isShot_ = false;
	//ドラッグ可能か
	bool ableDrag_ = true;
	// マウス感度
	float mouseInsensitivity_ = 0.020f;	
	// ドラッグ中かどうか
	bool dragging_ = false;	
	//グラスのラジアン
	float glassRadian_ = 0.0f;
	//グラススピードパワー
	float glassSpeedPower_ = 1.0f;
};

