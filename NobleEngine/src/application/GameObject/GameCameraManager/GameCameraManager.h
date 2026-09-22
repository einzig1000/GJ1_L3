#pragma once
#include<Game.h>

class GameCameraManager
{
public:
	GameCameraManager();
	~GameCameraManager();
	/// @brief ターゲット度数
	/// @param targetDeg 
	void Initialize(const float targetDeg);
	void ChangeCameraPhase(CameraPhase phase);
	void Update();
	/// @brief カメラフェーズの更新
    /// @param ableDrag ドラッグ可能か　参照渡しで書換えることもある
    /// @param glassTranslate グラスの位置
    /// @param glassVelocity グラスの速度
    /// @param mouseInsensitivity マウス感度
	void UpdateCameraPhase(
		bool& ableDrag,
		const Vector3& glassTranslate,
		const Vector3& glassVelocity,
		const float mouseInsensitivity);

	void SetClosestThetaRadian(const float targetDeg);
	int32_t GetCameraID() { return c_main_; }
	CameraPhase GetCameraPhase() { return cameraPhase_; }
	void SetPhi(const float phi) { cameraSpherical_.phi = phi; };
private:

	int32_t c_main_ = -1;
	Coordinate_spherical cameraSpherical_ = { 0.0f, 0.0f, 0.0f };
	CameraPhase cameraPhase_ = CameraPhase::Free;
	CounterSec cameraPhaseCounter_;


};

