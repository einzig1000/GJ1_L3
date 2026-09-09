#include <Game.h>

class CocktailWater
{
public:
	// VS b0 : 192 byte
	struct WaterTransformCB
	{
		Matrix4x4 world;
		Matrix4x4 worldInverseTranspose;
		Matrix4x4 viewProjection;
	};

	// VS b1 : 112 byte
	struct WaterWaveCB
	{
		// 大きくするとその軸の波が細かく多くなる
		Vector4 relativeScale{ 1.0f, 1.0f, 1.0f, 0.0f };
		// 波専用の座標軸。いじった人間から殺していく
		Vector4 waveAxisX{ 1.0f, 0.0f, 0.0f, 0.0f };
		Vector4 waveAxisY{ 0.0f, 1.0f, 0.0f, 0.0f };
		Vector4 waveAxisZ{ 0.0f, 0.0f, 1.0f, 0.0f };
		// 水面の高さ。手動でいじるものではない。
		float surfaceY = 0.0f;
		// 実質波の高さ。
		float commonWorldScale = 1.0f;
		// 
		float sideWaveDepth = 0.02f;
		// 真・波の高さ。大きくするとその軸の波が多くなる
		float waveHeight = 0.04f;
		// 波の周波数。
		float waveFrequency = 10.000f;
		// 波のスピード。実質timeScale
		float waveSpeed = 2.0f;
		// ↓の倍率。
		float motionHeightBoost = 2.0f;
		// グラスが今どれだけ激しく動いているか
		float motionIntensity = 0.0f;
		// 時間。motionWaveTime += deltaTimeMs * 0.001f;
		float motionWaveTime = 0.0f;
		float pad0 = 0.0f, pad1 = 0.0f, pad2 = 0.0f;
	};

	// PS b0 : 16 byte
	struct WaterCameraCB
	{
		Vector3 cameraPositionWS{};
		float   pad = 0.0f;
	};

	// PS b1 : 96 byte
	struct WaterColorCB
	{
		Vector4 colorA{ 1.0f, 0.0f, 0.0f, 0.65f };
		Vector4 colorB{ 0.0f, 0.0f, 1.0f, 0.65f };
		Vector4 baseColor{ 1.0f, 1.0f, 1.0f, 0.65f };

		float colorBalance = 0.5f;
		float colorBlendWidth = 0.25f;
		float colorDistortion = 0.30f;
		float colorPatternScale = 1.0f;

		float convectionSpeed = 1.0f;
		float convectionStrength = 0.4f;
		float convectionScale = 8.0f;
		float mixProgress = 0.0f;

		float motionIntensity = 0.0f;
		float motionWaveTime = 0.0f;
		float smoothness = 0.8f;
		float fresnelStrength = 0.3f;   // 0で元Unity版と同じ
	};

	// PS b2 : 64 byte
	struct WaterLightingCB
	{
		Vector4 mainLightDirection{ 0.0f, 1.0f, 0.0f, 0.0f };
		Vector4 mainLightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vector4 ambientSky{ 0.25f, 0.30f, 0.38f, 1.0f };
		Vector4 ambientGround{ 0.06f, 0.05f, 0.05f, 1.0f };
	};

	static_assert(sizeof(WaterTransformCB) == 192);
	static_assert(sizeof(WaterWaveCB) == 112);
	static_assert(sizeof(WaterCameraCB) == 16);
	static_assert(sizeof(WaterColorCB) == 96);
	static_assert(sizeof(WaterLightingCB) == 64);

public:
	CocktailWater();
	~CocktailWater();
	void Initialize();
	void Update(int32_t cameraID);
	void Draw(int32_t renderTargetID);
	void DrawImGui();

	void SetTranslate(const Vector3& translate) { waterTransform_.translate = translate; }

private:
	std::unique_ptr<RenderObject> water_;
	EulerTransforms waterTransform_;
	WaterTransformCB waterTransformCB_;
	WaterWaveCB waterWaveCB_;
	WaterCameraCB waterCameraCB_;
	WaterColorCB waterColorCB_;
	WaterLightingCB waterLightingCB_;
};
