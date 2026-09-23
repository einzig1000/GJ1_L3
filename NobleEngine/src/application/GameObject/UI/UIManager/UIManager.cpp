#include "UIManager.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>
#include<GameObject/UI/BreakEvaluation/BreakEvaluation.h>
#include<GameObject/UI/ShakeProgress/ShakeProgress.h>
#include<numbers>
#include<System/SESystem/GameSESystem/GameSESystem.h>
#include<GameObject/Effect/CoinParticle/CoinParticle.h>

namespace {
    //ボーナス定数
    constexpr int32_t shakeBonus = 5000;
    constexpr int32_t benefit = 500;
}

UIManager::UIManager()
{
    // カメラ
    uiCameraID_ = Game::Camera::AddCamera("UICamera");

    benefitNumMesh_ = std::make_unique<NumMeshs>();
    timeNumMesh_ = std::make_unique<NumMeshs>();

    timeAndMoneySignboard_ = std::make_unique<UIModel>();

    yenWord_ = std::make_unique<UIModel>();
    timeWord_ = std::make_unique<UIModel>();

    //破壊する
    breakEvaluation_ = std::make_unique<BreakEvaluation>();
    //シェイクプログレス
    shakeProgress_ = std::make_unique<ShakeProgress>();

    //コインパーティクル
    coinParticle_ = std::make_unique<CoinParticle>();

    modelIds_["timeAndMoney"] = {
     Game::Asset::Model::Load("assets/application/model/UI_TIme&Money/UI_TIme&Money.obj") ,
     Game::Asset::Texture::Load("assets/application/model/UI_TIme&Money/UI.png")
    };

    int32_t white1x1 = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");

    modelIds_["yen"] = {
    Game::Asset::Model::Load("assets/application/model/Yen/yen.obj") ,
    white1x1
    };
    modelIds_["time"] = {
    Game::Asset::Model::Load("assets/application/model/Time/time.obj") ,
    white1x1
    };



}

UIManager::~UIManager()
{

}

void UIManager::Initialize()
{
    //60.0f
    gameTimer_ = 60.0f;
    aniTimer_ = 0.0f;
    isDown_ = true;

    cameraOffset_ = 0.0f;

    const float pi = std::numbers::pi_v<float>;
    const float hPi = pi * 0.5f;
    float range = 8.0f;

    timeAndMoneySignboard_->Initialize(
        modelIds_["timeAndMoney"].model_,
        modelIds_["timeAndMoney"].texture_,
        EulerTransforms{ {1.0f,1.0f,1.0f} ,{ 0.0f,3.642f,0.0f }, { -range,0.0f,1.0f } });


    yenWord_->Initialize(
        modelIds_["yen"].model_,
        modelIds_["yen"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,0.0f,0.0f },{-1.67f,0.4f,-0.2f } },
        timeAndMoneySignboard_->GetWorldMatrixPtr()
    );

    timeWord_->Initialize(
        modelIds_["time"].model_,
        modelIds_["time"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f, 0.0f,0.0f },{ -1.32f,-0.63f,-0.2f} },
        timeAndMoneySignboard_->GetWorldMatrixPtr()
    );

    breakEvaluation_->Initialize();

    shakeProgress_->Initialize();

    benefitNumMesh_->Initialize(6, { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, {-0.28f,0.4f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    timeNumMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, { 0.8f,-0.63f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());

    coinParticle_->Initialize();

    Game::Camera::Setter::SetEnableControl(false, uiCameraID_);

    Game::Camera::Setter::SetDistanceTarget(12.0f, 0.0f, EaseType::LINEAR, uiCameraID_);
    Game::Camera::Setter::SetThetaTarget(hPi, 0.0f, EaseType::LINEAR, uiCameraID_);
    Game::Camera::Setter::SetPhiTarget(0.16f, 0.0f, EaseType::LINEAR, uiCameraID_);

    Game::Camera::Setter::SetCenter({ 0.0f,-3.0,9.0f }, 0.0f, EaseType::LINEAR, uiCameraID_);


    
}

void UIManager::Update()
{

    //ゲームタイマー
    gameTimer_ -= Game::Time::GetScaledDeltaTimeMs() * 0.001f;
    timeNumMesh_->SetValue(static_cast<int32_t>(gameTimer_));

    //アニメーションタイマー
    aniTimer_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    if (aniTimer_ >= 0.5f) {
        aniTimer_ = 0.0f;
        //トグルする
        isDown_ = !isDown_;
        float offset = 0.125f * powf(-1.0f, isDown_);
        cameraOffset_ = offset;
        Game::Camera::Setter::SetCenter({ 0.0f, -3.0f + offset, 9.0f }, 0.5f, EaseType::LINEAR, uiCameraID_);
    }


    Game::Camera::Update(uiCameraID_);

    //破壊判定オブジェクト
    breakEvaluation_->Update(uiCameraID_);

    timeAndMoneySignboard_->Update(uiCameraID_);


    shakeProgress_->Update(uiCameraID_);

    yenWord_->Update(uiCameraID_);
    timeWord_->Update(uiCameraID_);

    benefitNumMesh_->Update(uiCameraID_);
    timeNumMesh_->Update(uiCameraID_);



    if (breakEvaluation_->GetIsAddScore()) {
       
        int32_t tempBenefit = breakEvaluation_->GetBenefit();
        //ベネフィットを入れちゃおー
        benefitNumMesh_->AddValue(tempBenefit);
        if (tempBenefit > 0) {
            //破壊数に応じて加算する あるいはお客さんに届いた。
            GameSESystem::PlaySE(GameSESystem::MONEY, true);
            //パーティクル
            coinParticle_->Emit(tempBenefit * 0.1f);
        }

    }

    if (isHitCustomer_) {
        // あるいはお客さんに届いた。
        GameSESystem::PlaySE(GameSESystem::MONEY, true);
        //シェイク値によってボーナスをかけて　渡ったら規定値500円
        int32_t tempBenefit = shakeProgress_->GetShakeProgress() * shakeBonus + benefit;
        benefitNumMesh_->AddValue(tempBenefit);
        isHitCustomer_ = false;
        //お客様に提供されたらゼロに戻す
        shakeProgress_->SetShakeProgress(0.0f);
        //パーティクル
        coinParticle_->Emit(tempBenefit*0.1f);
    }

    //ブレイクカウントから利益までの位置をセットしたい。
    coinParticle_->SetPosition(breakEvaluation_->GetSignBoardPos(), benefitNumMesh_->GetWorldPos());
    //コインパーティクル
    coinParticle_->Update(uiCameraID_);
}

void UIManager::Draw(const int32_t uiRenderTextureID)
{

    timeAndMoneySignboard_->Draw(uiRenderTextureID);

    //シェイク
    shakeProgress_->Draw(uiRenderTextureID);

    breakEvaluation_->Draw(uiRenderTextureID);
    yenWord_->Draw(uiRenderTextureID);
    timeWord_->Draw(uiRenderTextureID);

    benefitNumMesh_->Draw(uiRenderTextureID);
    timeNumMesh_->Draw(uiRenderTextureID);


    coinParticle_->Draw(uiRenderTextureID);
}

void UIManager::DrawImGui()
{

    coinParticle_->DebugImGui();

    int i = 0;
    timeAndMoneySignboard_->DebugUI(i++);
    yenWord_->DebugUI(i++);
    timeWord_->DebugUI(i++);
  

    benefitNumMesh_->DrawImGui("benefit");
    timeNumMesh_->DrawImGui("timeMesh");

    breakEvaluation_->DebugImGui();

    shakeProgress_->DebugUI();


}

void UIManager::SetScore(float score)
{
    benefitNumMesh_->SetValue(static_cast<int32_t>(score));
}

void UIManager::AddScore(float score)
{
    benefitNumMesh_->AddValue(static_cast<int32_t>(score));
}

int32_t UIManager::GetScore() const
{
    return benefitNumMesh_->GetValue();
}

void UIManager::SetShakeProgress(const float shake)
{
    shakeProgress_->SetShakeProgress(shake);
}
