#include "GameManager.h"
#include <App.h>
#include <Utilities/Logger/Logger.h>
#include <Utilities/Json/JsonManager.h>
#include <GameManager/Phase/TitlePhase/TitlePhase.h>
#include <GameManager/Phase/GameScenePhase/GameScenePhase.h>
#include <GameManager/Phase/TestPhase/TestPhase.h>
#include <GameManager/Phase/SikouteiDevelopPhase/SikouteiDevelopPhase.h>
#include <GameManager/Phase/ResultPhase/ResultPhase.h>

//衝突判定テスト用フェーズ
#include <GameManager/Phase/CollisionTestPhase/CollisionTestPhase.h>

GameManager::GameManager() 
{
	JsonManager::LoadAll("assets/application/json");


	phaseMap_[Phase::Phase_Title] = std::make_unique<TitlePhase>();
	phaseMap_[Phase::Phase_Title]->SetContext(&phaseContext_);
	phaseMap_[Phase::Phase_GameScene] = std::make_unique<GameScenePhase>();
	phaseMap_[Phase::Phase_GameScene]->SetContext(&phaseContext_);
	phaseMap_[Phase::Phase_Test] = std::make_unique<TestPhase>();
	phaseMap_[Phase::Phase_Test]->SetContext(&phaseContext_);
	phaseMap_[Phase::Phase_SikouteiDevelop] = std::make_unique<SikouteiDevelopPhase>();
	phaseMap_[Phase::Phase_SikouteiDevelop]->SetContext(&phaseContext_);
	phaseMap_[Phase::Phase_CollisionTest] = std::make_unique<CollisionTestPhase>();
	phaseMap_[Phase::Phase_CollisionTest]->SetContext(&phaseContext_);
	phaseMap_[Phase::Phase_Result] = std::make_unique<ResultPhase>();
	phaseMap_[Phase::Phase_Result]->SetContext(&phaseContext_);

	phaseContext_.renderTargetIDs.resize(static_cast<size_t>(Phase::Phase_Max));



	Phase startUpPhase = Phase::Phase_Result;
	currentPhase_ = phaseMap_[startUpPhase].get();
	currentPhase_->Initialize();

	currentRenderTargetID_ = phaseContext_.renderTargetIDs[static_cast<size_t>(startUpPhase)];
	targetRenderTargetID_ = currentRenderTargetID_;

	maskTextureCreator_ = std::make_unique<CreateMaskTexture>();
	maskTextureCreator_->Initialize();
	maskRenderTargetID_ = maskTextureCreator_->GetMaskTextureID();




	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
	renderObject_->psoConfig_.ps = "assets/shaders/FullScreen/Mask.PS.hlsl";
	renderObject_->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
	renderObject_->SetupFromShaders();
}

GameManager::~GameManager()
{
}

void GameManager::Update()
{
	Phase nextPhase = currentPhase_->GetNextPhase();
	if (nextPhase != Phase::Phase_None)
	{
		// タイマー開始
		counterSec_.SetTargetTime(10.0f);
		// フラグ乱立
		phaseChanging_ = true;

		previousPhase_ = currentPhase_;


		maskTextureCreator_->Initialize();

		currentPhase_ = phaseMap_[nextPhase].get();
		currentPhase_->Initialize();

		targetRenderTargetID_ = phaseContext_.renderTargetIDs[static_cast<size_t>(nextPhase)];
	}


	currentPhase_->Update();
	if (phaseChanging_)
	{
		previousPhase_->Update();
		maskTextureCreator_->Update();

		if (counterSec_.CountUp())
		{
			phaseChanging_ = false;
			currentRenderTargetID_ = targetRenderTargetID_;
		}
	}

	maskTextureCreator_->Draw();
	maskTextureCreator_->DrawImGui();

	if (Game::IO::Key::IsJustPressed(VK_F11))
	{
		Game::Asset::RenderTexture::SaveAllRenderTextureToFile("generated/screenshots");
	}
}

void GameManager::Draw()
{
	currentPhase_->Draw();
	if (phaseChanging_)
	{
		previousPhase_->Draw();
	}

	Vector3 keyColor = { 1.0f, 1.0f, 0.0f };
	float keyThreshold = 0.01f;

	renderObject_->SetCBufferData(0, ShaderType::PixelShader, &currentRenderTargetID_);
	renderObject_->SetCBufferData(1, ShaderType::PixelShader, &targetRenderTargetID_);
	renderObject_->SetCBufferData(2, ShaderType::PixelShader, &maskRenderTargetID_);
	renderObject_->SetCBufferData(3, ShaderType::PixelShader, &keyColor);
	renderObject_->SetCBufferData(4, ShaderType::PixelShader, &keyThreshold);
	renderObject_->SetCBufferData(5, ShaderType::PixelShader, &maskUV);
	renderObject_->Draw(-1, phaseContext_.renderTargetIDs);
}

void GameManager::DrawImGui()
{
	currentPhase_->DrawImGui();

	ImGui::Begin("GameManager");
	ImGui::DragFloat2("scale", &maskUV.scale.x, 0.01f);
	ImGui::DragFloat2("translate", &maskUV.translate.x, 0.01f);
	ImGui::DragFloat("rotate", &maskUV.rotate, 0.01f);
	ImGui::End();
}



void GameManager::ChangePhase(Phase phase)
{
	currentPhase_ = phaseMap_[phase].get();
}
