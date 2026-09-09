#include "GameManager.h"
#include <App.h>
#include <Utilities/Logger/Logger.h>
#include <Utilities/Json/JsonManager.h>
#include <GameManager/Phase/TitlePhase/TitlePhase.h>
#include <GameManager/Phase/GameScenePhase/GameScenePhase.h>
#include <GameManager/Phase/TestPhase/TestPhase.h>
#include <GameManager/Phase/SikouteiDevelopPhase/SikouteiDevelopPhase.h>
#include <GameManager/Phase/CollisionTestPhase/CollisionTestPhase.h>
#include <GameManager/Phase/ResultPhase/ResultPhase.h>
#include <GameManager/Phase/Tutorial/TutorialPhase.h>

#include <GameManager/Phase/ResultPhase/ResultPhase.h>
GameManager::GameManager() 
{
	JsonManager::LoadAll("assets/application/json");


	phaseContext_.renderTargetIDs.resize(static_cast<size_t>(Phase::Phase_Max));

	maskTextureCreator_ = std::make_unique<CreateMaskTexture>();
	maskTextureCreator_->Initialize();
	maskRenderTargetID_ = maskTextureCreator_->GetMaskTextureID();

	Phase startUpPhase = Phase::Phase_Title;
	currentPhase_ = CreatePhase(startUpPhase);
	currentPhase_->SetContext(&phaseContext_);

	currentPhase_->Initialize();

	currentRenderTargetID_ = phaseContext_.renderTargetIDs[static_cast<size_t>(startUpPhase)];
	targetRenderTargetID_ = currentRenderTargetID_;





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
		previousPhase_ = std::move(currentPhase_);

		currentPhase_ = CreatePhase(nextPhase);
		currentPhase_->SetContext(&phaseContext_);
		currentPhase_->Initialize();


		// タイマー開始
		counterSec_.Initialize(10.0f);
		// フラグ乱立
		phaseChanging_ = true;


		maskTextureCreator_->Initialize();


		targetRenderTargetID_ = phaseContext_.renderTargetIDs[static_cast<size_t>(nextPhase)];
	}


	currentPhase_->Update();
	if (phaseChanging_)
	{
		previousPhase_->Update();
		maskTextureCreator_->Update();
		maskTextureCreator_->Draw();

		if (counterSec_.GetProgress() >= 1.0f)
		{
			phaseChanging_ = false;
			currentRenderTargetID_ = targetRenderTargetID_;
		}
	}


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

std::unique_ptr<IPhase> GameManager::CreatePhase(Phase phase)
{
	switch (phase)
	{
	case Phase::Phase_Test:
		return std::make_unique<TestPhase>();
		break;
	case Phase::Phase_Title:
		return std::make_unique<TitlePhase>();
		break;
	case Phase::Phase_SikouteiDevelop:
		return std::make_unique<SikouteiDevelopPhase>();
		break;
	case Phase::Phase_Result:
		return std::make_unique<ResultPhase>();
		break;
	case Phase::Phase_Tutorial:
		return std::make_unique<TutorialPhase>();
		break;
	default:
		break;
	}
}
