#pragma once

#include <GameManager/Phase/IPhase.h>
//前方宣言
class Glass;

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
private:
	// カメラID
	int32_t c_main_ = -1;

#pragma region
	//ゲームオブジェクト
	std::unique_ptr<Glass>glass_ = nullptr;

#pragma endregion
};

