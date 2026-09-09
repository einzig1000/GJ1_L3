#pragma once
#include <GameManager/Phase/IPhase.h>
#include <numbers>
class ResultPhase : public IPhase {

public:
	ResultPhase();
	~ResultPhase() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void DrawImGui() override;

	void ChangePhase(Phase phase) override { nextPhase_ = phase; }
	void SetIsWin(bool isWin) { isWin_ = isWin; }

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

	// ========================================
	// ResultRay（ResultPhase用ボリュームライト）
	// ========================================

	struct alignas(16) ResultRayCameraBuffer {
		Matrix4x4 viewProjection;
		Vector3 cameraPositionWS;
		float padding;
	};

	struct alignas(16) ResultRayObjectBuffer {
		Matrix4x4 world;
		Matrix4x4 worldToObject;
	};

	struct alignas(16) ResultRayMaterialBuffer {
		// ResultRayの光の色
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
		// ResultRayのローカル座標における-Y方向へ伸びる
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

	Model resultRayModel_;
	ResultRayCameraBuffer resultRayCameraBuffer_{};
	ResultRayObjectBuffer resultRayObjectBuffer_{};
	ResultRayMaterialBuffer resultRayMaterialBuffer_{};

	// ResultRayの位置
	Vector3 resultRayPosition_ = Vector3(-65.0f, 19.0f, -4.0f);

	// GoodでManの手が上がり、ResultRayが点灯し始める時刻（秒）
	static constexpr float kResultRayTurnOnGoodTime_ = 1.0f;
	// ResultRayが上から下へ伸び切るまでの時間（秒）
	static constexpr float kResultRayRevealDuration_ = 0.35f;
	float resultRayRevealElapsedTime_ = 0.0f;
	bool isResultRayTurnedOn_ = false;

	void Initialize_ResultRayModel();
	void Update_ResultRayAnimation(float deltaTime);
	void Update_ResultRayModel();
	void Draw_ResultRayModel();

	bool isWin_ = false;

	// カメラID
	int32_t c_main_ = -1;

	// ========================================
	// Win演出後のカメラ（値はここで調整可能）
	// ========================================

	// 看板前に置くカクテルの位置。
	// Space後のカメラは、この座標を中心に周回・接近する。
	Vector3 cocktailPosition_ = Vector3(-60.0f, 7.0f, -10.0f);
	Vector3 cocktailScale_ = Vector3(10.0f, 10.0f, 10.0f);
	// 2.5周を、TitlePhaseと同じ角速度で回す
	static constexpr float kResultCameraOrbitDuration_ = 5.0f;
	static constexpr float kResultCameraOrbitAngle_ = std::numbers::pi_v<float> * 5.0f;
	// 周回しながら縮める最終距離
	static constexpr float kResultCameraOrbitEndDistance_ = 8.0f;
	// 周回後、弧を描いて中央へ飛び込む時間と高さ
	static constexpr float kResultCameraDiveDuration_ = 0.6f;
	static constexpr float kResultCameraArcHeight_ = 6.0f;
	// 中央へ近づいた後に残す距離
	static constexpr float kResultCameraStopDistance_ = 0.5f;

	enum class ResultCameraState {
		WaitingForInput,
		Orbiting,
		Diving,
		Finished,
	};

	ResultCameraState resultCameraState_ = ResultCameraState::WaitingForInput;
	float resultCameraElapsedTime_ = 0.0f;
	float resultCameraStartTheta_ = 0.0f;
	float resultCameraStartPhi_ = 0.0f;
	float resultCameraStartDistance_ = 1.0f;

	static float EaseInOut01(float value);
	void AimCameraFromPosition(const Vector3& cameraPosition, const Vector3& target);
	void Start_ResultCameraAnimation();
	void Update_ResultCameraAnimation(float deltaTime);

	// ライト用定数バッファ
	LightBuffer lightBuffer_{};

	Model barModel_;
	Model signboardModel_;
	Model cocktailModel_;
	Model cocktailWaterModel_;

	WaterWaveBuffer waterWaveBuffer_{};
	WaterCameraBuffer waterCameraBuffer_{};
	WaterColorBuffer waterColorBuffer_{};
	WaterLightingBuffer waterLightingBuffer_{};
	float cocktailWaterAnimationTime_ = 0.0f;

	void Initialize_LightModel(Model& model);
	void Update_LightModel(Model& model);
	void Initialize_LightBuffer();
	void Initialize_CocktailWaterModel();
	void Update_CocktailWaterModel(float deltaTime);

	// ========================================
	// Man（スキニングアニメーション）
	// ========================================

	int32_t manModelID_ = -1;
	int32_t manTextureID_ = -1;
	int32_t manIdleAnimationID_ = -1;
	int32_t manWalkAnimationID_ = -1;
	int32_t manGoodAnimationID_ = -1;
	int32_t manBadAnimationID_ = -1;
	int32_t manCatchAnimationID_ = -1;
	int32_t manThrowAnimationID_ = -1;
	int32_t manCurrentAnimationID_ = -1;

	// ManはZ=-60から歩き始め、Z=-10で停止する
	static constexpr float kManStartZ_ = -108.0f;
	static constexpr float kManTargetZ_ = -18.0f;

	static constexpr float kManWalkSpeed_ = 10.0f;
	static constexpr float kManTurnDuration_ = 0.75f;
	static constexpr float kManGoodEndTime_ = 1.999f;

	// Lose演出用
	// man.gltf内の各クリップの実際の終了時刻より少しだけ手前で止める
	static constexpr float kManBadEndTime_ = 2.374f;
	static constexpr float kManCatchEndTime_ = 1.916f;
	static constexpr float kManThrowEndTime_ = 1.624f;
	static constexpr float kManBadApproachDistance_ = 5.0f;

	enum class ManWinState {
		Walking,
		TurningToCamera,
		PlayingGood,
		HoldingGood,
	};

	ManWinState manWinState_ = ManWinState::Walking;

	enum class ManLoseState {
		Walking,
		PlayingBad,
		PlayingCatch,
		PlayingThrow,
		HoldingThrow,
	};

	ManLoseState manLoseState_ = ManLoseState::Walking;
	bool isManWalking_ = true;
	float manTurnElapsedTime_ = 0.0f;
	float manTurnStartY_ = 0.0f;
	float manTurnTargetY_ = 0.0f;
	Vector3 manBadStartPosition_{};
	Vector3 manBadTargetPosition_{};
	float manBadStartRotationY_ = 0.0f;
	float manBadTargetRotationY_ = 0.0f;

	// ========================================
	// Lose時のカクテルグラス追従・投擲
	// ========================================

	// man.gltf内で左手に使われているジョイント名
	static constexpr const char* kManLeftHandJointName_ = "Hand_L";
	// 左手ジョイント基準でのグラス位置調整値。
	// 手首ではなく手のひらへ合わせたい場合は、この値を微調整する。
	Vector3 loseGlassHandLocalOffset_ = Vector3(0.0f, 0.0f, 0.0f);
	// Hand_Lが見つからなかった場合だけ使用するMan原点基準の予備位置
	Vector3 loseGlassFallbackOffset_ = Vector3(-4.0f, 16.0f, 1.0f);
	// 看板へグラスが衝突する位置
	Vector3 loseGlassSignboardHitPosition_ = Vector3(-74.0f, 18.0f, -2.0f);
	// Throw開始から左手の追従を解除するまでの時間（秒）
	static constexpr float kLoseGlassReleaseTime_ = 1.10f;
	// 左手から看板へ到達するまでの時間（秒）
	static constexpr float kLoseGlassFlightDuration_ = 0.45f;
	// 投擲軌道の高さ
	static constexpr float kLoseGlassThrowArcHeight_ = 3.0f;

	int32_t manLeftHandJointIndex_ = -1;
	Vector3 loseGlassTablePosition_{};
	Vector3 loseGlassThrowStartPosition_{};
	Vector3 cocktailRotation_{};
	bool isLoseGlassReleased_ = false;
	bool hasLoseGlassHitSignboard_ = false;

	std::unique_ptr<RenderObject> manObject_;
	std::unique_ptr<ComputeObject> manAnimationCompute_;

	EulerTransforms manTransform_{};
	Vector4 manColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	float manAnimationTime_ = 0.0f;

	int32_t manResultHeapSlot_ = -1;
	uint32_t manVertexCount_ = 0;
	SkinInstance manSkinInstance_;
	const ModelData* manModelData_ = nullptr;

	void Initialize_Man();
	void Update_Man();
	void Update_WinMan();
	void Update_LoseMan();
	Vector3 GetLoseGlassLeftHandPosition();
	void Update_LoseGlassAnimation();
	void Draw_Man();
	void DrawImGui_Models();

	void InitializeCommon();
	void InitializeWin();
	void InitializeLose();

	void UpdateCommon();
	void UpdateWin();
	void UpdateLose();

	void DrawCommon();
	void DrawWin();
	void DrawLose();
};
