#pragma once
#include <GameManager/Phase/IPhase.h>

class TestPhase :
	public IPhase
{
public:
	TestPhase();
	~TestPhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;
	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:
	// カメラID
	int32_t c_main_ = -1;
	// モデルID
	int32_t m_cube_ = -1;
	int32_t m_plane_ = -1;
	int32_t m_human_ = -1;
	// テクスチャID
	int32_t t_uvChecker_ = -1;
	int32_t t_monsterBall_ = -1;
	// アニメーションID
	int32_t a_sneakWalk_ = -1;



	void Initialize_SimpleModels();
	void Update_SimpleModels();
	void Draw_SimpleModels();
	void DrawImGui_SimpleModels();
	std::unique_ptr<RenderObject> simpleModels_;
	int32_t instanceCount_ = 10;
	// ディスクリプタヒープスロット
	int32_t worldMatrixHeapSlot_ = -1;
	int32_t colorHeapSlot_ = -1;
	int32_t textureIndexHeapSlot_ = -1;
	std::vector<EulerTransforms> transforms_;
	std::vector<Matrix4x4> worldMatrices_;
	std::vector<Vector4> colors_;
	std::vector<int32_t> textureIndices_;


	void Initialize_Sprite();
	void Update_Sprite();
	void Draw_Sprite();
	void DrawImGui_Sprite();
	std::unique_ptr<RenderObject> sprite_;
	EulerTransforms spriteTransform_;
	Vector4 spriteColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	int32_t spriteTexture_ = -1;

	void Initialize_Animation();
	void Update_Animation();
	void Draw_Animation();
	void DrawImGui_Animation();
	std::unique_ptr<RenderObject> animation_;
	std::unique_ptr<ComputeObject> animationCompute_;
	float animationTime_ = 0.0f;
	int32_t resultHeapSlot_ = -1;
	uint32_t numVertices_;
	SkinInstance skinInstance_;
	const ModelData* modelData_ = nullptr;
};