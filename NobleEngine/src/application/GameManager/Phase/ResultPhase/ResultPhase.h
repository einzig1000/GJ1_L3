#pragma once
#include <GameManager/Phase/IPhase.h>
class ResultPhase : public IPhase {

public:
	ResultPhase();
	~ResultPhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;

	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:
	bool isWin_;


};
