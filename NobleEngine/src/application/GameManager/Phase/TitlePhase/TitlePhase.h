#pragma once
#include <GameManager/Phase/IPhase.h>

class TitlePhase :
	public IPhase
{
public:
	TitlePhase();
	~TitlePhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:

};