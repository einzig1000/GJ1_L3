#include "TitlePhase.h"

#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
TitlePhase::TitlePhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("SimpleModels");
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 15.0f, -60.0f), 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetRotate(Vector3(std::numbers::pi_v<float>,0.0f, 0.0f), 0.0f, EaseType::IN_BACK, c_main_);


	// モデル
	ID_ = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");

	// テクスチャ
	t_uvChecker_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");
}

TitlePhase::~TitlePhase() {}

void TitlePhase::Initialize() {
	// フェーズ初期化
	nextPhase_ = Phase::Phase_None;

	Initialize_LightModels();
}

void TitlePhase::Update() {
	Game::Camera::Update(c_main_);

	Update_LightModels();
}

void TitlePhase::Draw() { Draw_LightModels(); }

void TitlePhase::DrawImGui() {}

void TitlePhase::Initialize_LightModels() {
	simpleModels_ = std::make_unique<RenderObject>();

	simpleModels_->psoConfig_.vs = "assets/shaders/LightModel/LightModel.VS.hlsl";

	simpleModels_->psoConfig_.ps = "assets/shaders/LightModel/LightModel.PS.hlsl";

	simpleModels_->SetupFromShaders();

	simpleModels_->modelID_ = ID_;

	simpleModels_->instanceNum_ = instanceCount_;

	worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();

	colorHeapSlot_ = Game::Resource::CreateDynamic();

	textureIndexHeapSlot_ = Game::Resource::CreateDynamic();

	transforms_.resize(instanceCount_, EulerTransforms());

	worldMatrices_.resize(instanceCount_, Matrix4x4());

	colors_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	textureIndices_.resize(instanceCount_, t_uvChecker_);

	// ========================================
	// Light Buffer
	// ========================================

	lightBuffer_ = {};

	// 環境光
	lightBuffer_.ambientColor = Vector3(0.15f, 0.15f, 0.15f);

	// 使用するライト数
	lightBuffer_.lightCount = 1;

	// ========================================
	// Directional Light
	// ========================================

	Light& directionalLight = lightBuffer_.lights[0];

	directionalLight.type = 0;

	directionalLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	directionalLight.intensity = 1.0f;

	// 光が進む方向
	directionalLight.direction = Vector3(0.0f, -1.0f, 1.0f);

	// Directionalでは使用しない
	directionalLight.position = Vector3(0.0f, 0.0f, 0.0f);

	// Directionalでは使用しない
	directionalLight.radius = 1.0f;

	// Directionalでは使用しない
	directionalLight.decay = 2.0f;

	// Directionalでは使用しない
	directionalLight.distance = 10.0f;

	// Directionalでは使用しない
	directionalLight.cosAngle = 0.8f;

	// Directionalでは使用しない
	directionalLight.cosFalloffStart = 0.9f;
}

void TitlePhase::Update_LightModels() {
	for (int32_t i = 0; i < instanceCount_; ++i) {
		worldMatrices_[i] = transforms_[i].GetWorldMatrix();
	}

	Game::Resource::UpdateData(worldMatrixHeapSlot_, worldMatrices_);

	Game::Resource::UpdateData(colorHeapSlot_, colors_);

	Game::Resource::UpdateData(textureIndexHeapSlot_, textureIndices_);

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);

	int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlot_);

	Vector2uint psHeapSlot{
	    Game::Resource::GetSRV(colorHeapSlot_),

	    Game::Resource::GetSRV(textureIndexHeapSlot_)};

	// VS b0
	simpleModels_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);

	// VS b1
	simpleModels_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);

	// PS b0
	simpleModels_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);

	// PS b1
	// LightBuffer全体を送る
	simpleModels_->SetCBufferData(1, ShaderType::PixelShader, &lightBuffer_);
}

void TitlePhase::Draw_LightModels() { simpleModels_->Draw(); }