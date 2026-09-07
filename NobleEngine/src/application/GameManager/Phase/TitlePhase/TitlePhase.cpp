#include "TitlePhase.h"

#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <cmath>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
TitlePhase::TitlePhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("SimpleModels");
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetPhiTarget(std::numbers::pi_v<float>, 0.0f, EaseType::IN_BACK, c_main_);

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

	Update_Animation();
	Update_LightModels();
}

void TitlePhase::Draw() { Draw_LightModels(); }

void TitlePhase::DrawImGui() {}

void TitlePhase::Initialize_Models(Model& model) {
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

	Initialize_IceTransforms();

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

void TitlePhase::Initialize_IceTransforms() {
	const Vector3 iceScale(0.2f, 0.2f, 0.2f);
	const Vector3 iceRotate(0.0f, 0.0f, 0.0f);
	const float counterClockwiseAngle = -std::numbers::pi_v<float> * 0.5f;
	const float rotationCos = std::cos(counterClockwiseAngle);
	const float rotationSin = std::sin(counterClockwiseAngle);

	// カクテル中央を基準に、完成位置を反時計回りへ90度回転する
	const auto rotateTargetPosition = [rotationCos, rotationSin](float x, float z) {
		const float relativeX = x - kIceTargetCenterX_;
		const float relativeZ = z - kIceTargetCenterZ_;

		const float rotatedX = relativeX * rotationCos - relativeZ * rotationSin;
		const float rotatedZ = relativeX * rotationSin + relativeZ * rotationCos;

		return Vector3(kIceTargetCenterX_ + rotatedX, kIceTargetHeightY_, kIceTargetCenterZ_ + rotatedZ);
	};

	// 最初はグラスの中で下から上へ縦一列に並べる
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		iceStartPositions_[i] = Vector3(kIceStartX_, kIceStartBottomY_ + static_cast<float>(i) * kIceVerticalSpacing_, kIceStartZ_);

		iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, iceStartPositions_[i]);
	}

	// 1～4番：奥側
	// 4個の中心がカクテル中央に合うよう、X方向へ均等配置する
	constexpr int32_t backStartIndex = 0;
	constexpr int32_t backIceCount = 4;
	const float backStartX = kIceTargetCenterX_ - (static_cast<float>(backIceCount - 1) * kIceHorizontalSpacing_ * 0.5f);
	const float backZ = kIceTargetCenterZ_ - kIceDepthSpacing_;

	for (int32_t i = 0; i < backIceCount; ++i) {
		const float x = backStartX + static_cast<float>(i) * kIceHorizontalSpacing_;

		iceTargetPositions_[backStartIndex + i] = rotateTargetPosition(x, backZ);
	}

	// 5～9番：真ん中
	constexpr int32_t middleStartIndex = 4;
	constexpr int32_t middleIceCount = 5;
	const float middleStartX = kIceTargetCenterX_ - (static_cast<float>(middleIceCount - 1) * kIceHorizontalSpacing_ * 0.5f);

	for (int32_t i = 0; i < middleIceCount; ++i) {
		const float x = middleStartX + static_cast<float>(i) * kIceHorizontalSpacing_;

		iceTargetPositions_[middleStartIndex + i] = rotateTargetPosition(x, kIceTargetCenterZ_);
	}

	// 10～11番：手前側
	constexpr int32_t frontStartIndex = 9;
	constexpr int32_t frontIceCount = 2;
	const float frontStartX = kIceTargetCenterX_ - (static_cast<float>(frontIceCount - 1) * kIceHorizontalSpacing_ * 0.5f);
	const float frontZ = kIceTargetCenterZ_ + kIceDepthSpacing_;

	for (int32_t i = 0; i < frontIceCount; ++i) {
		const float x = frontStartX + static_cast<float>(i) * kIceHorizontalSpacing_;

		iceTargetPositions_[frontStartIndex + i] = rotateTargetPosition(x, frontZ);
	}

	iceAnimationElapsedTime_ = 0.0f;
	isIceAnimationFinished_ = false;
	previousAnimationTime_ = std::chrono::steady_clock::now();
}

void TitlePhase::Update_Animation() {
	if (isIceAnimationFinished_) {
		return;
	}

	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	float deltaTime = std::chrono::duration<float>(currentTime - previousAnimationTime_).count();

	previousAnimationTime_ = currentTime;

	// デバッグ停止などで極端に大きな時間が入った場合の瞬間移動を防ぐ
	if (deltaTime > 0.1f) {
		deltaTime = 0.1f;
	}

	iceAnimationElapsedTime_ += deltaTime;

	float t = iceAnimationElapsedTime_ / kIceMoveDuration_;
	if (t >= 1.0f) {
		t = 1.0f;
		isIceAnimationFinished_ = true;
	}

	const Vector3 iceScale(0.2f, 0.2f, 0.2f);

	// カクテルへ到着した時点で、氷本体も反時計回りへ90度になる
	const float targetIceRotationY = -std::numbers::pi_v<float> * 0.5f;
	const Vector3 iceRotate(0.0f, targetIceRotationY * t, 0.0f);

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		// 開始位置から、それぞれの完成位置へ線形補間する
		const Vector3 position = iceStartPositions_[i] * (1.0f - t) + iceTargetPositions_[i] * t;

		iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, position);
	}

	// 移動中だけグラスを少し持ち上げ、+Z側にあるカクテルへ傾ける。
	// sin(pi * t)により、開始時と終了時は元の姿勢へ戻る。
	const float glassMotionAmount = std::sin(std::numbers::pi_v<float> * t);

	const Vector3 glassScale(10.0f, 10.0f, 10.0f);
	const Vector3 glassRotate(std::numbers::pi_v<float> / 6.0f * glassMotionAmount, 0.0f, 0.0f);
	const Vector3 glassPosition(-60.0f, 7.0f + kGlassLiftHeight_ * glassMotionAmount, -60.0f);

	glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);
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
