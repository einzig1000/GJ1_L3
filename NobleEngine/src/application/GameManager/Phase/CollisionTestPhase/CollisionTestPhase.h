#pragma once

#include <GameManager/Phase/IPhase.h>

//前方宣言
class Glass;
class Table;
class CollisionManager;

class CollisionTestPhase :
	public IPhase
{
public:

	CollisionTestPhase();
	~CollisionTestPhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:
	void InitGameObj();
	void UpdateGameObj(const int32_t cameraID);
	void DrawGameObj();
	void DrawImGuiObj();
	void CheckColliders();
private:
	// カメラID
	int32_t c_main_ = -1;

#pragma region
	//ゲームオブジェクト
	std::unique_ptr<Glass>glass_ = nullptr;
	std::unique_ptr<Table>table_ = nullptr;
#pragma endregion

#pragma region
	//コリジョン管理
	std::unique_ptr<CollisionManager>collisionManager_ = nullptr;
#pragma endregion
};

