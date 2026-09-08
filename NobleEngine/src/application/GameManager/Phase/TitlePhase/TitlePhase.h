#pragma once

#include <GameManager/Phase/IPhase.h>
#include <chrono>
#include <cstdint>
#include <numbers>

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

	struct alignas(16) WaterTransformBuffer {
		Matrix4x4 world;
		Matrix4x4 worldInverseTranspose;
		Matrix4x4 viewProjection;
	};

	struct alignas(16) WaterWaveBuffer {
		Vector4 relativeScale;
		Vector4 waveAxisXWS;
		Vector4 waveAxisYWS;
		Vector4 waveAxisZWS;
		float surfaceY;
		float commonWorldScale;
		float sideWaveDepth;
		float waveHeight;
		float waveFrequency;
		float waveSpeed;
		float motionHeightBoost;
		float motionIntensity;
		float motionWaveTime;
		float padding[3];
	};

	struct alignas(16) WaterCameraBuffer {
		Vector3 cameraPositionWS;
		float padding;
	};

	struct alignas(16) WaterColorBuffer {
		Vector4 colorA;
		Vector4 colorB;
		Vector4 baseColor;
		float colorBalance;
		float colorBlendWidth;
		float colorDistortion;
		float colorPatternScale;
		float convectionSpeed;
		float convectionStrength;
		float convectionScale;
		float mixProgress;
		float motionIntensity;
		float motionWaveTime;
		float smoothness;
		float fresnelStrength;
	};

	struct alignas(16) WaterLightingBuffer {
		Vector4 mainLightDirection;
		Vector4 mainLightColor;
		Vector4 ambientSky;
		Vector4 ambientGround;
	};

	enum class TitlePhaseSelection {
		Start,
		Select,
	};

	static const int32_t kMaxIceCount_ = 11;
	static const int32_t kTitleSelectCount_ = 2;

	// ========================================
	// Title Select
	// ========================================

	// カクテルと同じ通常スケール
	static constexpr float kTitleSelectNormalScale_ = 1.0f;

	// 選択中の拡大スケール
	static constexpr float kTitleSelectSelectedScale_ = 2.0f;

	int32_t selectedTitleIndex_ = 0;

	// ========================================
	// Ice Layout
	// ========================================

	// 最初に氷を縦一列で置くグラス側の基準位置
	static constexpr float kIceStartX_ = -60.0f;
	static constexpr float kIceStartBottomY_ = 4.5f;
	static constexpr float kIceStartZ_ = -60.0f;
	static constexpr float kIceVerticalSpacing_ = 0.2f;

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

	// 傾いたグラスからカクテルへ飛ぶときの放物線の高さ
	static constexpr float kIceArcHeight_ = 3.0f;

	// 最初にグラス・氷・ジンが定位置へ滑り込む時間（秒）
	static constexpr float kEntranceDuration_ = 1.0f;

	// 定位置からZ方向へ離しておく距離
	static constexpr float kEntranceZDistance_ = 15.0f;

	// 氷の移動前にグラスと氷を持ち上げる時間（秒）
	static constexpr float kPreLiftDuration_ = 1.0f;

	// 氷の移動前にグラスと氷を持ち上げる量
	static constexpr float kGlassLiftHeight_ = 4.0f;

	// 持ち上げ後、グラスと氷を一緒に傾ける時間（秒）
	static constexpr float kGlassTiltDuration_ = 0.75f;

	// カメラが氷用グラスへ向きを変える時間（秒）
	static constexpr float kGlassCameraLookDuration_ = 0.5f;

	// グラスと氷を傾けたまま停止する時間（秒）
	static constexpr float kGlassTiltHoldDuration_ = 0.5f;

	// カクテル側へ傾ける角度（+30度）
	static constexpr float kGlassTiltAngle_ = std::numbers::pi_v<float> / 6.0f;

	// 氷を入れ終わった後、グラスが元の位置へ戻る時間（秒）
	static constexpr float kGlassReturnDuration_ = 0.75f;

	// ジンが最大角度まで傾く時間（秒）
	static constexpr float kGinTiltDuration_ = 0.5f;

	// ジンが最大まで傾いた姿勢で停止する時間（秒）
	static constexpr float kGinTiltHoldDuration_ = 1.0f;

	// ジンが最大角度から元の姿勢へ戻る時間（秒）
	static constexpr float kGinReturnDuration_ = 0.5f;

	// カメラをジンへ向ける時間（秒）
	static constexpr float kGinCameraMoveDuration_ = 0.35f;

	// ジンが最大まで傾いた後、液体が現れるまでの時間（秒）
	static constexpr float kCocktailWaterScaleDuration_ = 0.5f;

	// ジンが元の姿勢へ戻ってから退場を始めるまでの待機時間（秒）
	static constexpr float kPostGinWaitDuration_ = 5.0f;

	// グラス・ジンの退場とTitleSelectの登場に掛ける時間（秒）
	static constexpr float kTitleSelectTransitionDuration_ = 1.0f;

	// グラスとジンをZ方向へ退場させる距離
	static constexpr float kExitZDistance_ = 15.0f;

	// TitleSelectを完成位置より下へ離しておく距離
	static constexpr float kTitleSelectRiseDistance_ = 20.0f;

	// 横視点と上側視点を切り替える時間（秒）
	static constexpr float kCameraMoveDuration_ = 1.0f;

	// 最初の少し上から斜め下へ見るカメラ角度（15度）
	static constexpr float kInitialCameraPhi_ = std::numbers::pi_v<float> / 12.0f;

	// ジン演出後、カクテルグラスを真上から見るカメラ距離
	static constexpr float kPostGinOverheadDistance_ = 12.0f;

	// 選択確定後、カクテルと氷を選択位置へ移動する時間（秒）
	static constexpr float kSelectionConfirmMoveDuration_ = 1.0f;

	// 横視点から真上へ上がりながら周回する時間（秒）
	static constexpr float kSelectionCameraRiseDuration_ = 4.0f;

	// 選択確定後にカクテルの周囲を2周する角度
	static constexpr float kSelectionCameraOrbitAngle_ = std::numbers::pi_v<float> * 4.0f;

	// 真上へ到達した後、カクテルへ近づく時間（秒）
	static constexpr float kSelectionCameraApproachDuration_ = 1.25f;

	// 選択演出開始時のカメラ距離
	static constexpr float kSelectionCameraStartDistance_ = 20.0f;

	// カクテルへ近づいた後に停止するカメラ距離
	static constexpr float kSelectionCameraStopDistance_ = 0.5f;

	// 選択演出の最終カメラ角度（真上90度）
	static constexpr float kSelectionCameraTopPhi_ = std::numbers::pi_v<float> / 2.0f;

	Vector3 iceStartPositions_[kMaxIceCount_]{};
	Vector3 iceTiltedStartPositions_[kMaxIceCount_]{};
	Vector3 iceTargetPositions_[kMaxIceCount_]{};

	float entranceElapsedTime_ = 0.0f;
	bool isEntranceFinished_ = false;

	float preLiftElapsedTime_ = 0.0f;
	bool isPreLiftFinished_ = false;

	float glassTiltElapsedTime_ = 0.0f;
	bool isGlassTiltFinished_ = false;

	float glassTiltHoldElapsedTime_ = 0.0f;
	bool isGlassTiltHoldFinished_ = false;

	float iceAnimationElapsedTime_ = 0.0f;
	bool isIceAnimationFinished_ = false;
	bool isCocktailCameraStarted_ = false;
	bool isSideCameraReturned_ = false;

	float glassReturnElapsedTime_ = 0.0f;
	bool isGlassReturnFinished_ = false;

	float ginAnimationElapsedTime_ = 0.0f;
	bool isGinCameraStarted_ = false;
	float postGinWaitElapsedTime_ = 0.0f;
	float postGinCameraElapsedTime_ = 0.0f;
	bool isPostGinOverheadCameraStarted_ = false;
	float titleSelectTransitionElapsedTime_ = 0.0f;
	bool isTitleSelectTransitionStarted_ = false;
	bool isTitleSelectInputEnabled_ = false;
	float cocktailWaterScaleElapsedTime_ = 0.0f;
	float cocktailWaterAnimationTime_ = 0.0f;
	bool isCocktailWaterAppearing_ = false;
	TitlePhaseSelection titlePhaseSelection_ = TitlePhaseSelection::Start;
	bool isSelectionConfirmed_ = false;
	float selectionConfirmElapsedTime_ = 0.0f;
	float selectionCameraElapsedTime_ = 0.0f;
	float selectionCameraTheta_ = 0.0f;
	float selectedCocktailTargetZ_ = kIceTargetCenterZ_;
	Vector3 presentationCameraPosition_{};
	bool isPresentationCameraPositionFixed_ = false;
	Vector3 glassCameraStartFocus_{};
	Vector3 ginCameraStartFocus_{};

	std::chrono::steady_clock::time_point previousAnimationTime_{};
	std::chrono::steady_clock::time_point previousSelectionAnimationTime_{};

	Model barModel_;
	Model glassModel_;
	Model iceModel_[kMaxIceCount_];
	Model CocktailModel_;
	Model cocktailWaterModel_;
	Model ginModel_;
	Model titleSelectModels_[kTitleSelectCount_];

	// カメラID
	int32_t c_main_ = -1;

	// ライト用定数バッファ
	LightBuffer lightBuffer_{};
	WaterWaveBuffer waterWaveBuffer_{};
	WaterCameraBuffer waterCameraBuffer_{};
	WaterColorBuffer waterColorBuffer_{};
	WaterLightingBuffer waterLightingBuffer_{};

	void Initialize_Models(Model& model);
	void Initialize_WaterModel();
	void Initialize_IceTransforms();
	void Update_TitleSelect();
	void Start_SelectedCocktailAnimation();
	void Update_SelectedCocktailAnimation();
	void Update_Model(Model& model);
	void Update_WaterModel();
	void Update_Animation();
	void AimCameraFromFixedPosition(const Vector3& cameraPosition, const Vector3& target);
	void Start_CocktailCameraAnimation();
	void Start_SideCameraAnimation();
	void Initialize_LightModels();
	void Update_LightModels();
	void Draw_LightModels();
};
