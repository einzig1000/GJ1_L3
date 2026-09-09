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
	int32_t s_TitleScene_ = 0;
	std::vector<int32_t> s_TitleScene_PlayIDs_;
	float volume = 0.0f;

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

	struct alignas(16) TitleRayCameraBuffer {
		Matrix4x4 viewProjection;
		Vector3 cameraPositionWS;
		float padding;
	};

	struct alignas(16) TitleRayObjectBuffer {
		Matrix4x4 world;
		Matrix4x4 worldToObject;
	};

	struct alignas(16) TitleRayMaterialBuffer {
		// TitleRayの光の色
		// RGB：光の色
		// A：通常の透明度ではなく、最終的な光量へ掛ける強度倍率
		Vector4 color;

		// ボリュームライト全体の明るさ
		// 大きくするほど光が強く表示される
		float intensity;

		// 円錐の先端側の太さ
		// シェーダー内で0.5倍されるため、実質的には直径として扱われる
		float tipRadius;

		// 円錐の末端側の太さ
		// シェーダー内で0.5倍されるため、実質的には直径として扱われる
		float endRadius;

		// 円錐状の光が伸びる長さ
		// TitleRayのローカル座標における-Y方向へ伸びる
		float coneLength;

		// 光を先端からどこまで表示するか
		// 0.0f：完全に非表示
		// 1.0f：末端まで完全に表示
		float reveal;

		// revealによる表示境界のぼかし幅
		// 小さいほど境界がくっきりし、大きいほど滑らかに消える
		float revealSoftness;

		// 円錐内部の光の密度
		// 大きくするほど光が濃く、不透明に近い見た目になる
		float density;

		// 円錐の中心部分の明るさ倍率
		// 外周よりも中心を明るく見せるために使用する
		float centerBrightness;

		// 円錐外周のぼかし幅
		// 小さいほど輪郭がくっきりし、大きいほど外周が滑らかに消える
		float edgeSoftness;

		// 円錐の先端から離れるほど暗くする強さ
		// 大きくするほど末端側が暗くなる
		float distanceFade;

		// レイマーチングで光の密度を調べる回数
		// 大きいほど滑らかになるが、描画負荷も高くなる
		// シェーダー内部では8～96の範囲に制限される
		int32_t stepCount;

		// 定数バッファを16バイト境界に合わせるための未使用領域
		float padding;
	};

	enum class TitlePhaseSelection {
		Start,
		Select,
	};

	static const int32_t kMaxIceCount_ = 11;
	static const int32_t kTitleSelectCount_ = 2;

	// PresentationIceDX12の挙動をTitlePhase内だけで完結させる氷の実行状態。
	struct IcePresentationState {
		Vector3 position{};
		Vector3 rotation{};
		Vector3 scale{0.2f, 0.2f, 0.2f};
		Vector3 velocity{};
		Vector3 sourceLocalPosition{};
		Vector3 sinkStartPosition{};
		Vector3 submergedPosition{};
		Vector3 submergedRotation{};
		Vector3 arrangeStartPosition{};
		Vector3 arrangeStartRotation{};
		Vector3 floatStartPosition{};
		Vector3 floatStartRotation{};
		float releaseDelay = 0.0f;
		float floatDelay = 0.0f;
		float floatDurationScale = 1.0f;
		float driveSpeedScale = 1.0f;
		float horizontalDrift = 0.0f;
		float sinkTimer = 0.0f;
		float arrangeTimer = 0.0f;
		float floatTimer = 0.0f;
		bool isFollowingSourceGlass = true;
		bool hasReleased = false;
		bool hasEnteredCocktailGlass = false;
		bool isPhysicsActive = false;
		bool hasSunkInCocktailGlass = false;
		bool isArranging = false;
		bool hasArranged = false;
		bool isFloating = false;
		bool hasReachedTarget = false;
	};

	// ========================================
	// Title Select
	// ========================================

	// カクテルと同じ通常スケール
	static constexpr float kTitleSelectNormalScale_ = 1.0f;

	// 選択中の拡大スケール
	static constexpr float kTitleSelectSelectedScale_ = 2.0f;

	// Select中に両方のTitleSelectを呼吸するように拡大縮小する設定
	static constexpr float kTitleSelectPulseAmplitude_ = 0.08f;
	static constexpr float kTitleSelectPulseDuration_ = 1.2f;

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

	// 2つのTitleSelectを、カクテル中央からZ方向へそれぞれ10離して配置する
	static constexpr float kTitleSelectDistanceFromCenterZ_ = 10.0f;
	static constexpr float kTitleSelectFirstZ_ = kIceTargetCenterZ_ - kTitleSelectDistanceFromCenterZ_;
	static constexpr float kTitleSelectSecondZ_ = kIceTargetCenterZ_ + kTitleSelectDistanceFromCenterZ_;

	// 同じ列に並ぶ氷同士の横間隔
	static constexpr float kIceHorizontalSpacing_ = 0.4f;

	// 奥・中央・手前の列間隔
	static constexpr float kIceDepthSpacing_ = 0.4f;

	// グラスからカクテルへ移動する時間（秒）
	static constexpr float kIceMoveDuration_ = 2.0f;

	// カクテルグラスを上面が開いた円筒として扱う当たり判定
	static constexpr float kCocktailCollisionRadius_ = 1.2f;
	static constexpr float kCocktailCollisionBottomLocalY_ = -0.5f;
	static constexpr float kCocktailCollisionTopLocalY_ = 1.8f;
	static constexpr float kIceCollisionHalfExtent_ = 0.1f;

	// PresentationIceの疑似物理・浮上設定
	static constexpr float kIceDriveAcceleration_ = 30.0f;
	static constexpr float kIceMaxDriveSpeed_ = 12.0f;
	static constexpr float kIceGravity_ = 3.0f;
	static constexpr float kIceBounce_ = 0.25f;
	static constexpr float kIceSlideRetention_ = 0.96f;
	static constexpr float kIceSeparationAcceleration_ = 1.8f;
	static constexpr float kIceMaximumSeparationSpeed_ = 0.65f;
	static constexpr float kIceArrangeDuration_ = 0.55f;
	static constexpr float kIceFloatDuration_ = 0.4f;
	static constexpr float kIceFloatInterval_ = 0.025f;
	static constexpr float kIceRollMultiplier_ = 2.0f;

	// 氷を同時に放さず、1個ずつ異なる動きで投入する設定
	static constexpr float kIceFirstReleaseDelay_ = 0.3f;
	// グラスの傾斜開始後、従来の放出開始時刻からさらに待つ時間
	static constexpr float kIceMoveStartDelay_ = 0.15f;
	static constexpr float kIceReleaseInterval_ = 0.03f;
	static constexpr float kIceSinkDuration_ = 0.45f;
	static constexpr float kIceSubmergedDepth_ = 0.55f;
	static constexpr float kIceSubmergedDepthVariation_ = 0.08f;
	static constexpr float kIceSubmergedScatterRadius_ = 0.75f;

	// 最初にグラス・氷・ジンが定位置へ滑り込む時間（秒）
	static constexpr float kEntranceDuration_ = 1.0f;

	// 3つのモデルが揃った状態を映像と同じ間だけ見せる
	static constexpr float kEntranceHoldDuration_ = 3.0f;

	// グラス・氷・ジンが寄る前にTitleRayを伸ばす時間（秒）
	static constexpr float kTitleRayRevealDuration_ = 1.0f;

	// Select中にTitleRayを伸ばす時間（通常の2倍速）
	static constexpr float kTitleSelectRayRevealDuration_ = kTitleRayRevealDuration_ * 0.5f;

	// カクテルグラスの真上に置くTitleRayの先端位置
	static constexpr float kTitleRayPositionX_ = -60.0f;
	static constexpr float kTitleRayPositionY_ = 19.0f;
	static constexpr float kTitleRayPositionZ_ = -55.0f;

	// 通常時とSelect中のTitleRay末端側の幅
	static constexpr float kTitleRayEndRadius_ = 5.0f;
	static constexpr float kTitleSelectRayEndRadius_ = 12.0f;

	// 定位置からZ方向へ離しておく距離
	static constexpr float kEntranceZDistance_ = 15.0f;

	// グラスを動かす前にカメラを準備する時間（秒）
	static constexpr float kPreLiftDuration_ = 1.0f;

	// 傾斜と同時にグラスと氷を持ち上げる量
	static constexpr float kGlassLiftHeight_ = 4.0f;

	// 傾斜と同時にグラスと氷をZのプラス方向へ動かす量
	static constexpr float kGlassTiltMoveZ_ = 3.0f;

	// グラスと氷を持ち上げながら同時に傾ける時間（秒）
	static constexpr float kGlassTiltDuration_ = 0.5f;

	// カメラが氷用グラスへ向きを変える時間（秒）
	static constexpr float kGlassCameraLookDuration_ = 0.5f;

	// グラスと氷を傾けたまま停止する時間（秒）
	static constexpr float kGlassTiltHoldDuration_ = 0.5f;

	// カクテル側へ傾ける角度
	static constexpr float kGlassTiltAngle_ = std::numbers::pi_v<float> * 2 / 3.0f;

	// 氷を入れ終わった後、グラスが元の位置へ戻る時間（秒）
	static constexpr float kGlassReturnDuration_ = 0.75f;

	// 氷が沈み切った俯瞰画を保ってからジンへ戻るまでの間
	static constexpr float kIceSettledHoldDuration_ = 1.25f;

	// ジンが最大角度まで傾く時間（秒）
	static constexpr float kGinTiltDuration_ = 0.75f;

	// 傾斜量1.0のときにジンを持ち上げる高さ
	static constexpr float kGinLiftHeight_ = 8.0f;

	// ジンが最大まで傾いた姿勢で停止する時間（秒）
	static constexpr float kGinTiltHoldDuration_ = 2.0f;

	// ジンが最大角度から元の姿勢へ戻る時間（秒）
	static constexpr float kGinReturnDuration_ = 0.5f;

	// 真上の液面を見せてから、水面下で氷の整列を始めるまでの間
	static constexpr float kIceFloatWaitDuration_ = 2.25f;

	// ジンが最大まで傾いた後、液体が現れるまでの時間（秒）
	static constexpr float kCocktailWaterScaleDuration_ = 0.5f;

	// グラス・ジンの退場とTitleSelectの登場に掛ける時間（秒）
	static constexpr float kTitleSelectTransitionDuration_ = 0.3f;

	// グラスとジンをZ方向へ退場させる距離
	static constexpr float kExitZDistance_ = 15.0f;

	// TitleSelectを完成位置より下へ離しておく距離
	static constexpr float kTitleSelectRiseDistance_ = 20.0f;

	// 横視点と上側視点を切り替える時間（秒）
	static constexpr float kCameraMoveDuration_ = 1.0f;

	// 最初の少し上から斜め下へ見るカメラ角度（15度）
	static constexpr float kInitialCameraPhi_ = std::numbers::pi_v<float> / 12.0f;

	// 氷の移動中に向かう、カクテル斜め上カメラ
	static constexpr float kIceTransferCameraPhi_ = std::numbers::pi_v<float> / 4.0f;
	static constexpr float kIceTransferCameraTheta_ = std::numbers::pi_v<float> / 4.0f;
	static constexpr float kIceTransferCameraDistance_ = 14.0f;
	// 最初の氷を放すまでにカメラ補間を完了させる
	static constexpr float kIceTransferCameraDuration_ = kIceFirstReleaseDelay_;

	// 氷投入後、ZとYを固定してカメラをXのプラス方向へ動かす量
	static constexpr float kGinCameraMoveX_ = 5.0f;
	static constexpr float kGinViewCameraDuration_ = 0.75f;
	// ジンを見る位置へカメラが引き終わってから、ジンを動かすまでの停止時間
	static constexpr float kGinCameraHoldDuration_ = 0.5f;

	// ジンの傾斜中に到達する、真上から30度傾いたカクテル注視角度
	static constexpr float kCocktailPeekCameraPhi_ = std::numbers::pi_v<float> / 3.0f;
	// カクテルを正面から見るY軸まわりの角度
	static constexpr float kCocktailFrontCameraTheta_ = 0.0f;
	// ジンを置いた後、上記の角度を維持したまま近づく距離と時間
	static constexpr float kCocktailPeekCameraDistance_ = 8.0f;
	static constexpr float kCocktailApproachCameraDuration_ = 2.0f;

	// 選択確定後、カクテルと氷を選択位置へ移動する時間（秒）
	static constexpr float kSelectionConfirmMoveDuration_ = 1.0f;

	// 高さと距離を維持し、カクテルの横を2周する時間（秒）
	static constexpr float kSelectionCameraOrbitDuration_ = 4.0f;

	// 選択確定後にカクテルの周囲を2周する角度
	static constexpr float kSelectionCameraOrbitAngle_ = std::numbers::pi_v<float> * 4.0f;

	// 横周回が終わった時点でのカクテルとの距離
	static constexpr float kSelectionCameraOrbitEndDistance_ = 8.0f;

	// 正面で周回を止めた後、真上へ移動しながら近づく時間（秒）
	static constexpr float kSelectionCameraTopMoveDuration_ = 1.4f;

	// 正面から真上へ移動する放物線の、直線軌道からの追加高度
	static constexpr float kSelectionCameraTopArcHeight_ = 12.0f;

	// カクテルへ近づいた後に停止するカメラ距離
	static constexpr float kSelectionCameraStopDistance_ = 0.5f;

	Vector3 iceStartPositions_[kMaxIceCount_]{};
	Vector3 iceTargetPositions_[kMaxIceCount_]{};
	IcePresentationState presentationIce_[kMaxIceCount_]{};
	Vector3 sourceGlassIcePosition_{};
	float sourceGlassIceRotationX_ = 0.0f;
	bool isIcePseudoPhysicsReleased_ = false;
	bool isIceSubmergeFinished_ = false;
	bool isIceFloatStarted_ = false;
	float iceOverheadCameraElapsedTime_ = 0.0f;
	float iceSettledHoldElapsedTime_ = 0.0f;
	float iceFloatWaitElapsedTime_ = 0.0f;

	float titleRayRevealElapsedTime_ = 0.0f;
	bool isTitleRayRevealFinished_ = false;
	float titleSelectRayRevealElapsedTime_ = 0.0f;
	bool isTitleSelectRayActive_ = false;
	// 選択変更時は、現在の光を下端から上端へ縮め切ってから
	// 新しい選択位置で上端から下端へ伸ばし直す。
	bool isTitleSelectRayClosing_ = false;
	float titleSelectRayCloseStartReveal_ = 0.0f;
	int32_t titleSelectRayDisplayIndex_ = 0;
	Vector3 titleRayPosition_{};

	float entranceElapsedTime_ = 0.0f;
	bool isEntranceFinished_ = false;
	float entranceHoldElapsedTime_ = 0.0f;
	bool isEntranceHoldFinished_ = false;

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
	bool isCocktailViewCameraStarted_ = false;
	bool isCocktailPeekCameraStarted_ = false;
	float titleSelectCameraElapsedTime_ = 0.0f;
	float titleSelectTransitionElapsedTime_ = 0.0f;
	bool isTitleSelectTransitionStarted_ = false;
	bool isTitleSelectInputEnabled_ = false;
	float titleSelectPulseElapsedTime_ = 0.0f;
	float cocktailWaterScaleElapsedTime_ = 0.0f;
	float cocktailWaterAnimationTime_ = 0.0f;
	bool isCocktailWaterAppearing_ = false;
	TitlePhaseSelection titlePhaseSelection_ = TitlePhaseSelection::Start;
	bool isSelectionConfirmed_ = false;
	float selectionConfirmElapsedTime_ = 0.0f;
	float selectionCameraElapsedTime_ = 0.0f;
	float selectionCameraTheta_ = 0.0f;
	float selectedCocktailTargetZ_ = kIceTargetCenterZ_;
	Vector3 selectionCameraFixedPosition_{};
	Vector3 presentationCameraPosition_{};
	bool isPresentationCameraPositionFixed_ = false;
	Vector3 glassCameraStartFocus_{};
	Vector3 iceTransferCameraStartFocus_{};
	Vector3 ginCameraStartFocus_{};

	std::chrono::steady_clock::time_point previousAnimationTime_{};
	std::chrono::steady_clock::time_point previousSelectionAnimationTime_{};

	Model barModel_;
	Model glassModel_;
	Model iceModel_[kMaxIceCount_];
	Model CocktailModel_;
	Model cocktailWaterModel_;
	Model titleRayModel_;
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
	TitleRayCameraBuffer titleRayCameraBuffer_{};
	TitleRayObjectBuffer titleRayObjectBuffer_{};
	TitleRayMaterialBuffer titleRayMaterialBuffer_{};

	void Initialize_Models(Model& model);
	void Initialize_WaterModel();
	void Initialize_TitleRayModel();
	void Initialize_IceTransforms();
	void Initialize_PresentationIce();
	void Update_IceSourceTransform(const Vector3& position, float rotationX);
	void Update_PresentationIce(float deltaTime, bool updatePhysics);
	void Update_PresentationIcePhysics(float deltaTime);
	void Apply_CocktailGlassCollision(IcePresentationState& ice);
	void Apply_IceSeparation(float deltaTime);
	void Update_IceFloat(IcePresentationState& ice, int32_t iceIndex, float deltaTime);
	void Update_IceWave(IcePresentationState& ice, int32_t iceIndex);
	void Sync_IceModelsFromPresentation();
	static float Clamp01(float value);
	static float EaseInOut01(float value);
	static float VectorLength(const Vector3& value);
	static Vector3 NormalizeVector(const Vector3& value);
	void Update_TitleSelect();
	void Start_SelectedCocktailAnimation();
	void Update_SelectedCocktailAnimation();
	void Update_Model(Model& model);
	void Update_WaterModel();
	void Update_TitleRayModel();
	void Update_Animation();
	void Update_Sound();
	void AimCameraFromFixedPosition(const Vector3& cameraPosition, const Vector3& target);
	void Start_CocktailCameraAnimation();
	void Start_GinCameraAnimation();
	void Start_CocktailViewCameraAnimation();
	void Start_CocktailPeekCameraAnimation();
	void Start_SideCameraAnimation();
	void Initialize_LightModels();
	void Update_LightModels();
	void Draw_LightModels();
};
