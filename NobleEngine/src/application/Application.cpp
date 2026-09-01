#include "Application.h"
#include <GameManager/GameManager.h>
#include <ResourceLoader/Data/DataManager.h>
#include <Editor/editor.h>

Application& Application::Instance()
{
	static Application instance;
	return instance;
}

void Application::Initialize()
{
	// Font読み込み
	Game::Asset::Font::Load("Assets/engine/fonts/DotGothic16/DotGothic16-Regular.ttf");
	

	gameManager_ = std::make_unique<GameManager>();
}

void Application::Update()
{
	gameManager_->Update();
}

void Application::Draw()
{
	gameManager_->Draw();
}

void Application::DrawImGui()
{
	gameManager_->DrawImGui();
}

void Application::Finalize()
{
	gameManager_.reset();
}
