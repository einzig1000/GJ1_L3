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
	
	std::unique_ptr<RenderObject> renderObject_;
	int32_t instanceCount_ = 10;

	// カメラID
	int32_t c_main_ = -1;

	// モデルID
	int32_t m_cube_ = -1;
	
	// テクスチャID
	int32_t t_uvChecker_ = -1;
	int32_t t_monsterBall_ = -1;

	// ディスクリプタヒープスロット
	int32_t worldMatrixHeapSlot_ = -1;
	int32_t colorHeapSlot_ = -1;
	int32_t textureIndexHeapSlot_ = -1;

	std::vector<EulerTransforms> transforms_;
	std::vector<Matrix4x4> worldMatrices_;
	std::vector<Vector4> colors_;
	std::vector<int32_t> textureIndices_;
};