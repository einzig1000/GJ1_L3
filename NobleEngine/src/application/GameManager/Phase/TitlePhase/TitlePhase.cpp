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
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
	glassModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Water/Water.obj");
	CocktailModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Cocktail/Cocktail.obj");	
	ginModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Gin/Gin.obj");

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		iceModel_[i].ID = Game::Asset::Model::Load("assets/application/model/Title_Ice/Title_Ice1.obj");
		iceModel_[i].textureID_ = Game::Asset::Texture::Load("assets/application/model/Title_Ice/Title_Ice.png");
	}

	// テクスチャ
	barModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");
	glassModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Water/Water.png");
	CocktailModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Cocktail/Cocktail.png");
	ginModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Gin/Gin.png");
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

void TitlePhase::Initialize_Models(Model& model){
	model.Models_ = std::make_unique<RenderObject>();

	model.Models_->psoConfig_.vs = "assets/shaders/LightModel/LightModel.VS.hlsl";
	model.Models_->psoConfig_.ps = "assets/shaders/LightModel/LightModel.PS.hlsl";

	model.Models_->SetupFromShaders();

	model.Models_->modelID_ = model.ID;

	model.Models_->instanceNum_ = model.instanceCount_;
	model.WorldMatrixHeapSlot_ = Game::Resource::CreateDynamic();

	model.ColorHeapSlot_ = Game::Resource::CreateDynamic();

	model.TextureIndexHeapSlot_ = Game::Resource::CreateDynamic();

	model.transforms_.resize(model.instanceCount_, EulerTransforms());

	model.worldMatrices_.resize(model.instanceCount_, Matrix4x4());

	model.colors_.resize(model.instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	model.textureIndices_.resize(model.instanceCount_, model.textureID_);
}

void TitlePhase::Initialize_LightModels() {

	Initialize_Models(barModel_);
	Initialize_Models(glassModel_);
	Initialize_Models(CocktailModel_);
	Initialize_Models(ginModel_);
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Initialize_Models(iceModel_[i]);
	}

	glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -60.0f));
	CocktailModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -45.0f));
	for (int i = 0; i < kMaxIceCount_; ++i) {
		iceModel_[i].transforms_[0] = EulerTransforms(Vector3(0.2f, 0.2f, 0.2f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 2.5f+0.4f*i, -60.0f));
	}

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

void TitlePhase::Update_Animation() {
}

void TitlePhase::Update_LightModels() {
	Update_Model(barModel_);
	Update_Model(glassModel_);
	Update_Model(CocktailModel_);
	Update_Model(ginModel_);
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Update_Model(iceModel_[i]);
	}
}

void TitlePhase::Update_Model(Model& model) {
	for (int32_t i = 0; i < model.instanceCount_; ++i) {
		model.worldMatrices_[i] = model.transforms_[i].GetWorldMatrix();
	}

	Game::Resource::UpdateData(model.WorldMatrixHeapSlot_, model.worldMatrices_);

	Game::Resource::UpdateData(model.ColorHeapSlot_, model.colors_);

	Game::Resource::UpdateData(model.TextureIndexHeapSlot_, model.textureIndices_);

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);

	int32_t vsHeapSlot = Game::Resource::GetSRV(model.WorldMatrixHeapSlot_);

	Vector2uint psHeapSlot{
	    Game::Resource::GetSRV(model.ColorHeapSlot_),

	    Game::Resource::GetSRV(model.TextureIndexHeapSlot_)};

	// VS b0
	model.Models_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);

	// VS b1
	model.Models_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);

	// PS b0
	model.Models_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);

	// PS b1
	// LightBuffer全体を送る
	model.Models_->SetCBufferData(1, ShaderType::PixelShader, &lightBuffer_);
}

void TitlePhase::Draw_LightModels() {
	barModel_.Models_->Draw();
	glassModel_.Models_->Draw();
	CocktailModel_.Models_->Draw();
	ginModel_.Models_->Draw();
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		iceModel_[i].Models_->Draw();
	}
}