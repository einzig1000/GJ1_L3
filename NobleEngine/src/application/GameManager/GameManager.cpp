#include "GameManager.h"
#include <App.h>
#include <Utilities/Logger/Logger.h>
#include <Utilities/Json/JsonManager.h>
#include <GameManager/Phase/TitlePhase/TitlePhase.h>
#include <GameManager/Phase/GameScenePhase/GameScenePhase.h>
#include <GameManager/Phase/TestPhase/TestPhase.h>
#include <GameManager/Phase/SikouteiDevelopPhase/SikouteiDevelopPhase.h>

//衝突判定テスト用フェーズ
#include <GameManager/Phase/CollisionTestPhase/CollisionTestPhase.h>

GameManager::GameManager() 
{
	currentPhase_ = CreatePhase(Phase::Phase_SikouteiDevelop);
	currentPhase_->SetContext(&phaseContext_);
	currentPhase_->Initialize();

	JsonManager::LoadAll("assets/application/json");
}

GameManager::~GameManager()
{
}

void GameManager::Update()
{
	if (currentPhase_->GetNextPhase() != Phase::Phase_None)
	{
		currentPhase_ = CreatePhase(currentPhase_->GetNextPhase());
		currentPhase_->SetContext(&phaseContext_);
		currentPhase_->Initialize();
	}
	currentPhase_->Update();

	if (Game::IO::Key::IsJustPressed(VK_F11))
	{
		Game::Asset::RenderTexture::SaveAllRenderTextureToFile("generated/screenshots");
	}
}

void GameManager::Draw()
{
	currentPhase_->Draw();

}

void GameManager::DrawImGui()
{
	currentPhase_->DrawImGui();

}


std::unique_ptr<IPhase> GameManager::CreatePhase(Phase phase)
{
	switch (phase)
	{
	case Phase::Phase_Test:
		return std::make_unique<TestPhase>();
	case Phase::Phase_Title:
		return std::make_unique<TitlePhase>();
	case Phase::Phase_GameScene:
		return std::make_unique<GameScenePhase>();
	case Phase::Phase_CollisionTest:
		return std::make_unique<CollisionTestPhase>();
	case Phase::Phase_SikouteiDevelop:
		return std::make_unique<SikouteiDevelopPhase>();
	default:
		Log("Error : 該当するフェーズクラスが存在しません");
		assert(false);
		return nullptr;
	}
}