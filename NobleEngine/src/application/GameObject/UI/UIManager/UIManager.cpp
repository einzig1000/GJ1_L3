#include "UIManager.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>

UIManager::UIManager()
{
    // カメラ
    uiCameraID_ = Game::Camera::AddCamera("UICamera");

    benefitMesh_ = std::make_unique<NumMeshs>();
    timeMesh_ = std::make_unique<NumMeshs>();

    timeAndMoneySignboard_ = std::make_unique<UIModel>();
    breakSignboard_ = std::make_unique<UIModel>();
    cockTailSignboard_ = std::make_unique<UIModel>();

    yenWorld_ = std::make_unique<UIModel>();
    timeWorld_ = std::make_unique<UIModel>();
    breakWorld_ = std::make_unique<UIModel>();


    modelIds_["timeAndMoney"] = {
     Game::Asset::Model::Load("assets/application/model/UI_TIme&Money/UI_TIme&Money.obj") ,
     Game::Asset::Texture::Load("assets/application/model/UI_TIme&Money/UI.png")
    };

    modelIds_["breakSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_break/UI_break.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_break//UI.png")
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

    modelIds_["break"] = {
    Game::Asset::Model::Load("assets/application/model/Break/break.obj") ,
    white1x1
    };

}

UIManager::~UIManager()
{

}

void UIManager::Initialize()
{

    const float windowWidthSize = static_cast<float>(Game::Window::GetWidth());
    const float windowHeightSize = static_cast<float>(Game::Window::GetHeight());

    benefitMesh_->Initialize(6, { windowWidthSize-128.0f-64.0f-32.0f,128.0f-16.0f,-10.0f }, { 0.0f,3.14f,3.14f }, {48.0f,48.0f,48.0f});
    timeMesh_->Initialize(2, { windowWidthSize - 128.0f - 64.0f  +16.0f+16.0f+16.0f,128.0f + 16.0f+8.0f,-10.0f}, {0.0f,3.14f,3.14f}, {64.0f,64.0f,64.0f});

    timeAndMoneySignboard_->Initialize(
        modelIds_["timeAndMoney"].model_,
        modelIds_["timeAndMoney"].texture_,
        { windowWidthSize-128.0f-64.0f,128.0f,0.0f }, { 0.1f,0.0f,3.14f }, {48.0f,48.0f,48.0f});

    breakSignboard_->Initialize(
        modelIds_["breakSignboard"].model_,
        modelIds_["breakSignboard"].texture_,
        { windowWidthSize * 0.5f,128.0f,0.0f }, { 0.0f,0.0f,3.14f }, { 48.0f,48.0f,48.0f });

    cockTailSignboard_->Initialize(
        modelIds_["cocktailSignboard"].model_,
        modelIds_["cocktailSignboard"].texture_,
        {128.0f,128.0f,0.0f }, { 0.1f,0.125f,3.14f }, { 48.0f,48.0f,48.0f });

    yenWorld_->Initialize(
        modelIds_["yen"].model_,
        modelIds_["yen"].texture_,
        { windowWidthSize - 256.0f-32.0f+16.0f,128.0f-16.0f,-10.0f }, { 0.1f,3.14f,3.14f }, { 48.0f,48.0f,48.0f });


    timeWorld_->Initialize(
        modelIds_["time"].model_,
        modelIds_["time"].texture_,
        { windowWidthSize - 128.0f-64.0f-32.0f-16.0f,128.0f+16.0f+8.0f,-10.0f }, { 0.1f,3.14f,3.14f }, { 48.0f,48.0f,48.0f });



    breakWorld_->Initialize(
        modelIds_["break"].model_,
        modelIds_["break"].texture_,
        { windowWidthSize*0.5f,128.0f-16.0f-16.0f,-10.0f }, { 0.1f,3.14f,3.14f }, { 48.0f,48.0f,48.0f });


}

void UIManager::Update()
{
    //Game::Camera::Setter::SetCenter();
    Game::Camera::Update(uiCameraID_);

    timeAndMoneySignboard_->Update(uiCameraID_);
    breakSignboard_->Update(uiCameraID_);
    cockTailSignboard_->Update(uiCameraID_);

    yenWorld_->Update(uiCameraID_);
    timeWorld_->Update(uiCameraID_);
    breakWorld_->Update(uiCameraID_);

    benefitMesh_->Update(uiCameraID_);
    timeMesh_->Update(uiCameraID_);
}

void UIManager::Draw()
{
 
    timeAndMoneySignboard_->Draw();
    breakSignboard_->Draw();
    cockTailSignboard_->Draw();
    
    yenWorld_->Draw();
    timeWorld_->Draw();
    breakWorld_->Draw();
    benefitMesh_->Draw();
    timeMesh_->Draw();

}

void UIManager::DebugImGui()
{
    benefitMesh_->DrawImGui();
}
