#include "UIManager.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>
#include<GameObject/UI/BreakEvaluation/BreakEvaluation.h>
#include<numbers>

UIManager::UIManager()
{
    // カメラ
    uiCameraID_ = Game::Camera::AddCamera("UICamera");

    benefitMesh_ = std::make_unique<NumMeshs>();
    timeMesh_ = std::make_unique<NumMeshs>();

    timeAndMoneySignboard_ = std::make_unique<UIModel>();

    cockTailSignboard_ = std::make_unique<UIModel>();

    yenWorl_ = std::make_unique<UIModel>();
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

    renderTextureID_ = Game::Asset::RenderTexture::CreateRenderTexture(Game::Window::GetWidth(), Game::Window::GetHeight(),"UIRender");
}

UIManager::~UIManager()
{

}

void UIManager::Initialize()
{
    //60.0f
    gameTimer_ = 60.0f;

    float pi = 3.14159265358979f;
    float range = 8.0f;

    timeAndMoneySignboard_->Initialize(
        modelIds_["timeAndMoney"].model_,
        modelIds_["timeAndMoney"].texture_,
        EulerTransforms{ {1.0f,1.0f,1.0f} ,{ 0.0f,3.642f,0.0f }, { -range,0.0f,1.0f } });

    cockTailSignboard_->Initialize(
        modelIds_["cocktailSignboard"].model_,
        modelIds_["cocktailSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,2.562f,0.0f }, { range,0.0f,1.0f } });

    yenWorl_->Initialize(
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
 /*   breakEvaluation_->SetBreakCount();
    breakEvaluation_->SetMaxBreakCount();*/

    benefitMesh_->Initialize(6, { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, {-0.28f,0.4f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    timeMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, { 0.8f,-0.63f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    
    isInitializeCamera_ = false;

    const float hPi = std::numbers::pi_v<float>*0.5f;
    Game::Camera::Setter::SetPhiTarget(hPi, 0.0f, EaseType::LINEAR, uiCameraID_);
    Game::Camera::Setter::SetCenter({ 0.0f,-1.3,11.9f }, 0, EaseType::LINEAR, uiCameraID_);
    Game::Camera::Update(uiCameraID_);
}

void UIManager::Update()
{

    if (!isInitializeCamera_) {
        isInitializeCamera_ = true;
        const float hPi = std::numbers::pi_v<float>*0.5f;
        Game::Camera::Setter::SetPhiTarget(hPi, 0.0f, EaseType::LINEAR, uiCameraID_);
        Game::Camera::Setter::SetCenter({ 0.0f,-1.3,11.9f }, 0, EaseType::LINEAR, uiCameraID_);
        Game::Camera::Update(uiCameraID_);
    }

    //ゲームタイマー
    gameTimer_ -= Game::Time::GetScaledDeltaTimeMs()*0.001f;
    timeMesh_->SetValue(gameTimer_);
  /*  Game::Camera::Update(uiCameraID_);*/
    //Game::Camera::Setter::SetCenter();

    //破壊判定オブジェクト
    breakEvaluation_->Update(uiCameraID_);

    timeAndMoneySignboard_->Update(uiCameraID_);

    cockTailSignboard_->Update(uiCameraID_);

    yenWorl_->Update(uiCameraID_);
    timeWord_->Update(uiCameraID_);
 

    benefitMesh_->Update(uiCameraID_);
    timeMesh_->Update(uiCameraID_);

}

void UIManager::Draw()
{
 
    timeAndMoneySignboard_->Draw(renderTextureID_);

    cockTailSignboard_->Draw(renderTextureID_);
    
    breakEvaluation_->Draw(renderTextureID_);

    yenWorl_->Draw(renderTextureID_);
    timeWord_->Draw(renderTextureID_);
  
    benefitMesh_->Draw(renderTextureID_);
    timeMesh_->Draw(renderTextureID_);

}

void UIManager::DebugImGui()
{

    int i = 0;
    timeAndMoneySignboard_->DebugUI(i++);
   
    cockTailSignboard_->DebugUI(i++);
    yenWorl_->DebugUI(i++);
    timeWord_->DebugUI(i++);

    benefitMesh_->DrawImGui("benefit");
    timeMesh_->DrawImGui("timeMesh");

    breakEvaluation_->DebugImGui();
}

void UIManager::SetScore(float score)
{
    benefitMesh_->SetValue(static_cast<int32_t>(score));
}

void UIManager::AddScore(float score)
{
	benefitMesh_->AddValue(static_cast<int32_t>(score));
}

int32_t UIManager::GetScore() const
{
    return benefitMesh_->GetValue();
}
