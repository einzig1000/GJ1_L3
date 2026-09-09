#pragma once
#include <memory>
#include <unordered_map>
#include <GameManager/Phase/IPhase.h>
#include <System/CreateMaskTexture/CreateMaskTexture.h>

struct MaskUVTransform
{
	Vector2 scale = { 1.0f, 1.0f };
	Vector2 translate = { 0.0f, 0.0f };
	float rotate = 0.0f;
	float _pad[3] = {}; // HLSL側の16バイト境界パディング分の安全マージン
};
class GameManager
{
public:
	GameManager();
	~GameManager();

	void Update();
	void Draw();
	void DrawImGui();


private:

	std::unique_ptr<IPhase> previousPhase_;
	std::unique_ptr<IPhase> currentPhase_;

	std::unique_ptr<IPhase> CreatePhase(Phase phase);
	PhaseContext phaseContext_;
	



	bool phaseChanging_ = false;



	MaskUVTransform maskUV{};


	std::unique_ptr<RenderObject> renderObject_;
	int32_t currentRenderTargetID_ = -1;
	int32_t targetRenderTargetID_ = -1;
	int32_t maskRenderTargetID_ = -1;
	CounterSec counterSec_;
	Vector4 maskUVTransform_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::unique_ptr<CreateMaskTexture> maskTextureCreator_;
};

