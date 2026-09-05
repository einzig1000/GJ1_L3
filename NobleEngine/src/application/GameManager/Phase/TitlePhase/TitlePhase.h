#pragma once

#include <GameManager/Phase/IPhase.h>
#include <cstdint>

class TitlePhase : public IPhase {
public:
	TitlePhase();
	~TitlePhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;

	void ChangePhase(Phase phase) override { nextPhase_ = phase; }

private:
	static constexpr int32_t kMaxLightCount_ = 20;

	void Initialize_LightModels();
	void Update_LightModels();
	void Draw_LightModels();

	// ========================================
	// Light
	// ========================================

	struct alignas(16) Light {
		// 0 = Directional
		// 1 = Point
		// 2 = Spot
		// 3 = Area
		int32_t type;

		// typeを16バイト単位に揃える
		float padding0[3];

		// ライトの色
		Vector4 color;

		// ライトの強さ
		float intensity;

		// intensityを16バイト単位に揃える
		float padding1[3];

		// 光が進む方向
		Vector3 direction;

		// directionを16バイト単位に揃える
		float padding2;

		// 光源のワールド座標
		Vector3 position;

		// 光源付近の減衰調整
		float radius;

		// 距離減衰の指数
		float decay;

		// ライトが届く最大距離
		float distance;

		// スポットライト外側の角度のcos値
		float cosAngle;

		// スポットライトの減衰開始角度のcos値
		float cosFalloffStart;
	};

	struct alignas(16) LightBuffer {
		// ライト本体
		Light lights[kMaxLightCount_];

		// 使用するライト数
		int32_t lightCount;

		// lightCountを16バイト単位に揃える
		float padding0[3];

		// 環境光
		Vector3 ambientColor;

		// ambientColorを16バイト単位に揃える
		float padding1;
	};

	std::unique_ptr<RenderObject> simpleModels_;

	// カメラID
	int32_t c_main_ = -1;

	// モデルID
	int32_t ID_ = -1;

	// インスタンス数
	int32_t instanceCount_ = 1;

	// ディスクリプタヒープスロット
	int32_t worldMatrixHeapSlot_ = -1;

	// カラーヒープスロット
	int32_t colorHeapSlot_ = -1;

	// テクスチャインデックスヒープスロット
	int32_t textureIndexHeapSlot_ = -1;

	// インスタンスごとのトランスフォーム
	std::vector<EulerTransforms> transforms_;

	// インスタンスごとのワールド行列
	std::vector<Matrix4x4> worldMatrices_;

	// インスタンスごとのカラー
	std::vector<Vector4> colors_;

	// インスタンスごとのテクスチャインデックス
	std::vector<int32_t> textureIndices_;

	// テクスチャID
	int32_t t_uvChecker_ = -1;

	// ライト用定数バッファ
	LightBuffer lightBuffer_{};
};