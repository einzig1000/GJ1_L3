#include "GameScreen.h"

GameScreen::GameScreen()
{

    // レンダーターゲット
    renderTextureIDs_[MAIN_SCREEN] = Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(), 
        Game::Window::GetHeight(),
        "gameScene_3D",
        Vector4{ 0.0f,0.0f,0.0f,0.0f }
    );

    renderTextureIDs_[UI_SCREEN]= Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(),
        Game::Window::GetHeight(),
        "UIRender"
    );

    drawForMain_[MAIN_SCREEN] = std::make_unique<RenderObject>();
    drawForMain_[MAIN_SCREEN]->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
    drawForMain_[MAIN_SCREEN]->psoConfig_.ps = "assets/shaders/FullScreen/CopyImage.PS.hlsl";
    drawForMain_[MAIN_SCREEN]->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
    drawForMain_[MAIN_SCREEN]->SetupFromShaders();
    drawForMain_[MAIN_SCREEN]->SetCBufferData(0, ShaderType::PixelShader, &renderTextureIDs_[MAIN_SCREEN]);

    drawForMain_[UI_SCREEN] = std::make_unique<RenderObject>();
    drawForMain_[UI_SCREEN]->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
    drawForMain_[UI_SCREEN]->psoConfig_.ps = "assets/shaders/FullScreen/CopyImage.PS.hlsl";
    drawForMain_[UI_SCREEN]->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
    drawForMain_[UI_SCREEN]->SetupFromShaders();
    drawForMain_[UI_SCREEN]->SetCBufferData(0, ShaderType::PixelShader, &renderTextureIDs_[UI_SCREEN]);
}

void GameScreen::Draw(const int32_t renderTargetID)
{
    drawForMain_[0]->Draw(renderTargetID, { renderTextureIDs_[MAIN_SCREEN] });
    drawForMain_[1]->Draw(renderTargetID, { renderTextureIDs_[UI_SCREEN] });
}
