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

	// 現在はWin演出を先に実装しているため、単体確認時はWinを既定にする
	bool isWin_ = true;

	// カメラID
	int32_t c_main_ = -1;

	// ライト用定数バッファ
	LightBuffer lightBuffer_{};

	Model barModel_;

	void Initialize_LightModel(Model& model);
	void Update_LightModel(Model& model);
	void Initialize_LightBuffer();

	// ========================================
	// Man（スキニングアニメーション）
	// ========================================

	int32_t manModelID_ = -1;
	int32_t manTextureID_ = -1;
	int32_t manIdleAnimationID_ = -1;
	int32_t manWalkAnimationID_ = -1;
	int32_t manGoodAnimationID_ = -1;
	int32_t manCurrentAnimationID_ = -1;

	// ManはZ=-60から歩き始め、Z=-10で停止する
	static constexpr float kManStartZ_ = -85.0f;
	static constexpr float kManTargetZ_ = -15.0f;

	static constexpr float kManWalkSpeed_ = 10.0f;
	static constexpr float kManTurnDuration_ = 0.75f;
	static constexpr float kManGoodEndTime_ = 1.999f;

	enum class ManWinState {
		Walking,
		TurningToCamera,
		PlayingGood,
		HoldingGood,
	};

	ManWinState manWinState_ = ManWinState::Walking;
	bool isManWalking_ = true;
	float manTurnElapsedTime_ = 0.0f;
	float manTurnStartY_ = 0.0f;
	float manTurnTargetY_ = 0.0f;

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
