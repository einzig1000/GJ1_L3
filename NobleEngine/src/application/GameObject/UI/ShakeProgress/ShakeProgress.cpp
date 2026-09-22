#include "ShakeProgress.h"
#include<GameObject/UI/UIModel/UIModel.h>

namespace {
    const float glassTopY = 0.14f;
    const float glassBottomY = 0.23f;
}

ShakeProgress::ShakeProgress()
{

    textureIDs_.maskTexture_ = Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(),
        Game::Window::GetHeight(),
        "Glass_MaskTexture");
    
    //textureIDs_.texture_ = Game::Asset::RenderTexture::CreateRenderTexture(
    //    Game::Window::GetWidth(),
    //    Game::Window::GetHeight(),
    //    "ShakeProgressTexture");

    cockTailSignboard_ = std::make_unique<UIModel>();
    //グラス
    glassModel_ = std::make_unique<UIModel>();

    modelIds_["cocktailSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_cocktail/UI_cocktail.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_cocktail/UI.png")
    };

    int32_t white1x1 = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");

    modelIds_["glass"] = {
    Game::Asset::Model::Load("assets/application/model/Cocktail/Cocktail.obj") ,
    white1x1
    };

    renderObject_ = std::make_unique<RenderObject>();
    renderObject_->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
    renderObject_->psoConfig_.ps = "assets/shaders/GlassDissolve/GlassDissolve.PS.hlsl";
    renderObject_->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
    renderObject_->SetupFromShaders();


    shake_.glassTopY = glassTopY;
    shake_.glassBottomY = glassBottomY;
}

ShakeProgress::~ShakeProgress()
{
}

void ShakeProgress::Initialize()
{
    float range = 8.0f;
    cockTailSignboard_->Initialize(
        modelIds_["cocktailSignboard"].model_,
        modelIds_["cocktailSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,2.562f,0.0f }, { range,0.0f,1.0f } });
   
    //グラスモデル
    glassModel_->Initialize(
        modelIds_["glass"].model_,
        modelIds_["glass"].texture_,
        EulerTransforms{ { 3.0f,3.0f,3.0f } , { 0.0f, 0.0f,0.0f },{ -0.18f,0.18f,-1.0f} },
        cockTailSignboard_->GetWorldMatrixPtr()
    );
}

void ShakeProgress::Update(const int32_t uiCameraID)
{
    cockTailSignboard_->Update(uiCameraID);
    glassModel_->Update(uiCameraID);

    Vector3 edgeColor = { 1.0f,0.0f,0.0f };
    renderObject_->SetCBufferData(0, ShaderType::PixelShader, &textureIDs_.maskTexture_);
    renderObject_->SetCBufferData(1, ShaderType::PixelShader, &shake_);
    renderObject_->SetCBufferData(2, ShaderType::PixelShader, &edgeColor);
}

void ShakeProgress::Draw(const int32_t uiRenderTextureID)
{
    cockTailSignboard_->Draw(uiRenderTextureID);

    glassModel_->Draw(textureIDs_.maskTexture_);

    renderObject_->Draw(uiRenderTextureID);
}

void ShakeProgress::DebugUI()
{
    glassModel_->DebugUI(20);
    cockTailSignboard_->DebugUI(21);

    ImGui::Begin("UI");
    ImGui::SliderFloat("shakeProgress", &shake_.shakeProgress, 0.0f, 1.0f);
    ImGui::End();
}
