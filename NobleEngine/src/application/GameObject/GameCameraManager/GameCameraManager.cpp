#include "GameCameraManager.h"
#include<System/GameFunction/GameFunction.h>

GameCameraManager::GameCameraManager()
{    // カメラ
    c_main_ = Game::Camera::AddCamera("GamePhaseCamera");
}

GameCameraManager::~GameCameraManager()
{
}

void GameCameraManager::Initialize(const float targetDeg)
{
    cameraSpherical_.phi = 20.0f;
    Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.2f, EaseType::LINEAR, c_main_);
    cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, targetDeg);
    Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.2f, EaseType::OUT_BACK, c_main_);

}

void GameCameraManager::ChangeCameraPhase(CameraPhase phase)
{
    switch (phase)
    {
    case CameraPhase::Free:
    {
        Game::Camera::Setter::SetDistanceTarget(5.0f, 0.7f, EaseType::OUT_CIRC, c_main_);

        break;
    }
    case CameraPhase::ShotAngleSetup:
    {
        break;
    }
    case CameraPhase::GlassFollowing:
    {
        Game::Camera::Setter::SetDistanceTarget(1.0f, 0.2f, EaseType::LINEAR, c_main_);
        Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.2f, EaseType::LINEAR, c_main_);

        break;
    }
    case CameraPhase::CatchFollowing:
    {
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.7f, EaseType::OUT_CIRC, c_main_);

        Game::Camera::Setter::SetDistanceTarget(5.0f, 0.7f, EaseType::OUT_CIRC, c_main_);

        cameraPhaseCounter_.Initialize(0.7f);

        break;
    }
    default:
        break;
    }
    cameraPhase_ = phase;

}

void GameCameraManager::Update()
{    //カメラの更新
    Game::Camera::Update(c_main_);


}

void GameCameraManager::SetClosestThetaRadian(const float targetDeg)
{
    cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, targetDeg);

}

void GameCameraManager::UpdateCameraPhase(bool& ableDrag,const Vector3& glassTranslate,const Vector3&glassVelocity,const float mouseInsensitivity)
{
    switch (cameraPhase_)
    {
    case CameraPhase::Free:
    {
        // マウス移動量
        const Vector2 mouseDelta = Game::IO::Mouse::Get2DPositionDelta();
        constexpr float limit = 60.0f;

        break;
    }
    case CameraPhase::ShotAngleSetup:
    {
        // マウス移動量
        const Vector2 mouseDelta = Game::IO::Mouse::Get2DPositionDelta();
        constexpr float limit = 60.0f;

        // thetaをマウスで操作

        cameraSpherical_.theta -= mouseDelta.x * mouseInsensitivity;
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.0f, EaseType::OUT_BACK, c_main_);


        break;
    }
    case CameraPhase::GlassFollowing:
    {
        Vector3 glassVel = glassVelocity;
        Vector3 dir = Vector3(glassVel.x, 0.0f, glassVel.z).Normalized();
        //float theta = std::atan2(-dir.z, -dir.x); // 進行方向の逆(背後)
        float targetDeg = Game::Math::Converter::RadianToDegree(std::atan2(-dir.z, -dir.x));

        cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, targetDeg);

        // thetaをグラス後方で固定
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.5f, EaseType::OUT_BACK, c_main_);


        break;
    }
    case CameraPhase::CatchFollowing:
    {
        if (cameraPhaseCounter_.GetProgress() >= 1.0f)
        {
            ableDrag = true;
            ChangeCameraPhase(CameraPhase::Free);
        }

        break;
    }
    default:
        break;
    }

    // centerをグラスで固定
    Game::Camera::Setter::SetCenter(glassTranslate, 0.0f, EaseType::OUT_BACK, c_main_);

}
