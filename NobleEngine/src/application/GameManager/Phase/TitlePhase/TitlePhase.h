#pragma once

#include <GameManager/Phase/IPhase.h>
#include <chrono>
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

	struct Model {
		std::unique_ptr<RenderObject> Models_;
		int32_t ID;
		// インスタンス数
		int32_t instanceCount_ = 1;
		// ディスクリプタヒープスロット
		int32_t WorldMatrixHeapSlot_ = -1;

		// カラーヒープスロット
		int32_t ColorHeapSlot_ = -1;

		// テクスチャインデックスヒープスロット
		int32_t TextureIndexHeapSlot_ = -1;

		// インスタンスごとのトランスフォーム
		std::vector<EulerTransforms> transforms_;

		// インスタンスごとのワールド行列
		std::vector<Matrix4x4> worldMatrices_;

		// インスタンスごとのカラー
		std::vector<Vector4> colors_;

		// インスタンスごとのテクスチャインデックス
		std::vector<int32_t> textureIndices_;

		// テクスチャID
		int32_t textureID_ = -1;
	};

	static const int32_t kMaxIceCount_ = 11;

	// ========================================
	// Ice Layout
	// ========================================

	// 最初に氷を縦一列で置くグラス側の基準位置
	static constexpr float kIceStartX_ = -60.0f;
	static constexpr float kIceStartBottomY_ = 2.5f;
	static constexpr float kIceStartZ_ = -60.0f;
	static constexpr float kIceVerticalSpacing_ = 0.4f;

	// 最後に氷を面状に並べるカクテル側の基準位置
	static constexpr float kIceTargetCenterX_ = -60.0f;
	static constexpr float kIceTargetHeightY_ = 7.5f;
	static constexpr float kIceTargetCenterZ_ = -55.0f;

	// 同じ列に並ぶ氷同士の横間隔
	static constexpr float kIceHorizontalSpacing_ = 0.4f;

	// 奥・中央・手前の列間隔
	static constexpr float kIceDepthSpacing_ = 0.4f;

	// グラスからカクテルへ移動する時間（秒）
	static constexpr float kIceMoveDuration_ = 2.0f;

	// 氷の移動中にグラスを持ち上げる最大量
	static constexpr float kGlassLiftHeight_ = 1.0f;

	Vector3 iceStartPositions_[kMaxIceCount_]{};
	Vector3 iceTargetPositions_[kMaxIceCount_]{};

	float iceAnimationElapsedTime_ = 0.0f;
	bool isIceAnimationFinished_ = false;
	std::chrono::steady_clock::time_point previousAnimationTime_{};

	Model barModel_;
	Model glassModel_;
	Model iceModel_[kMaxIceCount_];
	Model CocktailModel_;
	Model ginModel_;

	// カメラID
	int32_t c_main_ = -1;

	// ライト用定数バッファ
	LightBuffer lightBuffer_{};

	void Initialize_Models(Model& model);
	void Initialize_IceTransforms();
	void Update_Model(Model& model);
	void Update_Animation();
	void Initialize_LightModels();
	void Update_LightModels();
	void Draw_LightModels();
};
