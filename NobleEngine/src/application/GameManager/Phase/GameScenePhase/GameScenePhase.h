#pragma once
#include <GameManager/Phase/IPhase.h>
#include <memory>

class GameScenePhase :
	public IPhase
{
public:
	GameScenePhase();
	~GameScenePhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:

};