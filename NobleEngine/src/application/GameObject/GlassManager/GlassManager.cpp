#include "GlassManager.h"
#include <GameObject/CocktailWater/CocktailWater.h>
#include <GameObject/Glass/Glass.h>
#include<System/GameFunction/GameFunction.h>
//予測オブジェクト
#include <GameObject/PredictionObj/PredictionObj.h>
//ゲームカメラ
#include<GameObject/GameCameraManager/GameCameraManager.h>
#include<System/CollisionManager/CollisionManager.h>
#include<algorithm>
#include<System/SESystem/GameSESystem/GameSESystem.h>

namespace {
    constexpr Vector3 kCocktailOffset = { 0.0f,-0.09f,0.0f };
    constexpr float kPowerScale = 0.05f; // 感度。要調整
    constexpr float kMaxSpeed = 5.0f;   // 上限。要調整
}

GlassManager::GlassManager()
{
    //カクテル流体
    cocktailWater_ = std::make_unique<CocktailWater>();
    //グラス
    glass_ = std::make_unique<Glass>();
    //予測線
    prediction_ = std::make_unique<PredictionObj>();
}

GlassManager::~GlassManager()
{

}

void GlassManager::SetLightData(LightDataForGPU* data)
{
    //グラス
    glass_->SetLightData(data);
    //予測線
    prediction_->SetLightData(data);
}

void GlassManager::SetCollisionManager(CollisionManager* collisionManager)
{
    //予測線
    prediction_->SetCollisionManager(collisionManager);
}

void GlassManager::Initialize()
{
    //ショットフラグ
    isShot_ = false;
    //グラス
    glass_->Initialize();
    //カクテル
    cocktailWater_->Initialize();
    //ベロシティの初期化をする
    glass_->SetVelocity(Vector3{});
    //予測オブジェクト
    prediction_->Initialize();
}

void GlassManager::Update(const int32_t cameraID)
{
    glass_->Update(cameraID);
    cocktailWater_->SetTranslate(glass_->GetTranslate() + kCocktailOffset);
    cocktailWater_->Update(cameraID);
}

void GlassManager::PlayerControl(GameCameraManager* gameCameraManager, std::vector<std::unique_ptr<TableObject>>& tebleObjects)
{
    if (ableDrag_)
    {
        if (Game::IO::Mouse::IsJustPressed(0))
        {
            dragStartPos_ = Game::IO::Mouse::Get2DPosition();
            velocity_ = Vector2(0.0f, 0.0f);
            gameCameraManager->ChangeCameraPhase(CameraPhase::ShotAngleSetup);
            dragging_ = true;
        }

        if (dragging_ && Game::IO::Mouse::IsHeld(0))
        {
            Vector2 dragVector = Game::IO::Mouse::Get2DPosition() - dragStartPos_;
            float dragLengthY = dragStartPos_.y - Game::IO::Mouse::Get2DPosition().y;

            if (dragLengthY > 0.0f)
            {
                Vector3 cameraDir = Game::Camera::Getter::GetCameraDirection(gameCameraManager->GetCameraID());
                cameraDir.y = 0.0f;
                cameraDir.Normalize();

                float power = std::clamp(dragLengthY * kPowerScale, 0.0f, kMaxSpeed);
                velocity_ = Vector2(cameraDir.x, cameraDir.z) * power;
            } else
            {
                velocity_ = Vector2(0.0f, 0.0f);
            }

            Vector3 velocity = { velocity_.x, 0.0f, velocity_.y };

            prediction_->SetVelocity(velocity);
            prediction_->SetTranslate(glass_->GetTranslate());
            //予測線の更新をする
            prediction_->Update(gameCameraManager->GetCameraID(), tebleObjects);
        }

        if (dragging_ && Game::IO::Mouse::IsJustReleased(0))
        {
            dragging_ = false;
            if (velocity_.LengthSq() > 0.5f)
            {
                glass_->SetVelocity(Vector3(velocity_.x, 0.0f, velocity_.y));
                gameCameraManager->SetPhi(20.0f);

                ableDrag_ = false;
                prediction_->SetVelocity(Vector3{});
                //
                isShot_ = true;
                gameCameraManager->ChangeCameraPhase(CameraPhase::GlassFollowing);
            } else
            {
                gameCameraManager->ChangeCameraPhase(CameraPhase::Free);
            }
        }
    }
}

void GlassManager::DrawPrediction(const int32_t renderTexture)
{
    //ショットアングルセットアップ時に描画する
    prediction_->Draw(renderTexture);

}

void GlassManager::Draw(const int32_t renderTexture)
{
    //カクテル液体の描画
    cocktailWater_->Draw(renderTexture);
    //グラスは半透明なので後に描画する
    glass_->Draw(renderTexture);
}

void GlassManager::DrawImGui()
{
    glass_->DrawImGui();
    cocktailWater_->DrawImGui();
    prediction_->DrawImGui();

    ImGui::Begin("Editor");
    ImGui::Checkbox("ableDrag", &ableDrag_);
    ImGui::End();
}

void GlassManager::SetPosForTableAndHuman(const Vector3& tablePos, const float tableRadius, const float humanDeg, const float tabelHeight)
{
    //ここで0.5をかける
    Vector3 glassPos = GameFunction::GetPositionOnCircle(tablePos, tableRadius * 0.5f, humanDeg);
    //テーブル高さの設定をここでするが、どうしようか
    glassPos.y = tabelHeight;
    glass_->SetTranslate(glassPos);
}

void GlassManager::SetShakeValue(const float shake)
{
    cocktailWater_->SetMixProgress(shake);
}

void GlassManager::AddShakeValue(const float shake)
{
   const float temp = cocktailWater_->GetMixProgress();
   float value = std::clamp(temp + shake, 0.0f, 1.0f);
   cocktailWater_->SetMixProgress(value);
}

float GlassManager::GetShakeValue()
{
    return cocktailWater_->GetMixProgress();
}

Glass* GlassManager::GetGlassPtr() { return glass_.get(); }

const float GlassManager::GetMouseInsensitivity()
{
    return mouseInsensitivity_;
}
