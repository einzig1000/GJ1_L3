#include "UIManager.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>
#include<GameObject/UI/BreakEvaluation/BreakEvaluation.h>
#include<numbers>
#include<System/SESystem/GameSESystem/GameSESystem.h>

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

    cockTailSignboard_ = std::make_unique<UIModel>();

    yenWord_ = std::make_unique<UIModel>();
    timeWord_ = std::make_unique<UIModel>();

    breakEvaluation_ = std::make_unique<BreakEvaluation>();

    modelIds_["timeAndMoney"] = {
     Game::Asset::Model::Load("assets/application/model/UI_TIme&Money/UI_TIme&Money.obj") ,
     Game::Asset::Texture::Load("assets/application/model/UI_TIme&Money/UI.png")
    };

    modelIds_["cocktailSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_cocktail/UI_cocktail.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_cocktail/UI.png")
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

    const uint32_t windowWidthSize =Game::Window::GetWidth();
    const uint32_t windowHeightSize = Game::Window::GetHeight();


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

    const float pi = std::numbers::pi_v<float>;
    const float hPi = pi *0.5f;
    float range = 8.0f;

    timeAndMoneySignboard_->Initialize(
        modelIds_["timeAndMoney"].model_,
        modelIds_["timeAndMoney"].texture_,
        EulerTransforms{ {1.0f,1.0f,1.0f} ,{ 0.0f,3.642f,0.0f }, { -range,0.0f,1.0f } });

    cockTailSignboard_->Initialize(
        modelIds_["cocktailSignboard"].model_,
        modelIds_["cocktailSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,2.562f,0.0f }, { range,0.0f,1.0f } });

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

    benefitNumMesh_->Initialize(6, { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, {-0.28f,0.4f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    timeNumMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, { 0.8f,-0.63f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());

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

    if (aniTimer_>= 0.5f) {
        aniTimer_ = 0.0f;
        //トグルする
        isDown_ = !isDown_;
        float offset = 0.125f* powf(-1.0f,isDown_);
        Game::Camera::Setter::SetCenter({ 0.0f, -3.0f+ offset, 9.0f}, 0.5f, EaseType::LINEAR, uiCameraID_);
    }

     Game::Camera::Update(uiCameraID_);
    
    //破壊判定オブジェクト
    breakEvaluation_->Update(uiCameraID_);

    timeAndMoneySignboard_->Update(uiCameraID_);
    cockTailSignboard_->Update(uiCameraID_);

    yenWord_->Update(uiCameraID_);
    timeWord_->Update(uiCameraID_);

    benefitNumMesh_->Update(uiCameraID_);
    timeNumMesh_->Update(uiCameraID_);

    if (breakEvaluation_->GetIsAddScore()) {
        //破壊数に応じて加算する あるいはお客さんに届いた。
        GameSESystem::PlaySE(GameSESystem::MONEY,true);
        //ベネフィットを入れちゃおー
        benefitNumMesh_->AddValue(breakEvaluation_->GetBenefit());
    }

    if (isHitCustomer_) {
        // あるいはお客さんに届いた。
        GameSESystem::PlaySE(GameSESystem::MONEY,true);
        //シェイク値によってボーナスをかけて　渡ったら規定値500円
        benefitNumMesh_->AddValue(shakeProgress_* shakeBonus + benefit);
        isHitCustomer_ = false;
        //お客様に提供されたらゼロに戻す
        shakeProgress_ = 0.0f;
    }

}

void UIManager::Draw(const int32_t uiRenderTextureID)
{
 
    timeAndMoneySignboard_->Draw(uiRenderTextureID);
    cockTailSignboard_->Draw(uiRenderTextureID);
    
    breakEvaluation_->Draw(uiRenderTextureID);

    yenWord_->Draw(uiRenderTextureID);
    timeWord_->Draw(uiRenderTextureID);
  
    benefitNumMesh_->Draw(uiRenderTextureID);
    timeNumMesh_->Draw(uiRenderTextureID);

}

void UIManager::DrawImGui()
{

    int i = 0;
    timeAndMoneySignboard_->DebugUI(i++);
   
    cockTailSignboard_->DebugUI(i++);
    yenWord_->DebugUI(i++);
    timeWord_->DebugUI(i++);

    benefitNumMesh_->DrawImGui("benefit");
    timeNumMesh_->DrawImGui("timeMesh");

    breakEvaluation_->DebugImGui();

    ImGui::Begin("UI");
    ImGui::SliderFloat("shakeProgress", &shakeProgress_, 0.0f, 1.0f);
    ImGui::End();
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
