#include "TitlePhase.h"

#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <algorithm>
#include <cmath>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
#include <string>
TitlePhase::TitlePhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("SimpleModels");
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetDistance(20.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetFovTarget(0.65f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetEnableControl(false, c_main_);

	// モデル
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
	glassModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Water/Water.obj");
	CocktailModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Cocktail/Cocktail.obj");
	cocktailWaterModel_.ID = Game::Asset::Model::Load("assets/application/model/Water/CocktailWater.obj");
	titleRayModel_.ID = Game::Asset::Model::Load("assets/application/model/Title_Select/TitleRay.obj");
	ginModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Gin/Gin.obj");
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		const std::string titleSelectModelPath = "assets/application/model/Title_Select/Title_Select" + std::to_string(i + 1) + ".obj";

		titleSelectModels_[i].ID = Game::Asset::Model::Load(titleSelectModelPath.c_str());
	}

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		// iceModel_[0]にはTitle_Ice1.obj、
		// iceModel_[10]にはTitle_Ice11.objを順番に割り当てる
		const std::string iceModelPath = "assets/application/model/Title_Ice/Title_Ice" + std::to_string(i + 1) + ".obj";

		iceModel_[i].ID = Game::Asset::Model::Load(iceModelPath.c_str());
		iceModel_[i].textureID_ = Game::Asset::Texture::Load("assets/application/model/Title_Ice/Title_Ice.png");
	}

	// テクスチャ
	barModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");
	glassModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Water/Water.png");
	CocktailModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Cocktail/Cocktail.png");
	ginModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Gin/Gin.png");
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		titleSelectModels_[i].textureID_ = Game::Asset::Texture::Load("assets/application/model/Title_Select/Title_Select.png");
	}
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
	Update_TitleSelect();
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

void TitlePhase::Initialize_WaterModel() {
	cocktailWaterModel_.Models_ = std::make_unique<RenderObject>();
	cocktailWaterModel_.Models_->psoConfig_.vs = "assets/shaders/WaterSurface/WaterSurface.VS.hlsl";
	cocktailWaterModel_.Models_->psoConfig_.ps = "assets/shaders/WaterSurface/WaterSurface.PS.hlsl";
	cocktailWaterModel_.Models_->SetupFromShaders();
	cocktailWaterModel_.Models_->modelID_ = cocktailWaterModel_.ID;
	cocktailWaterModel_.Models_->instanceNum_ = 1;
	cocktailWaterModel_.transforms_.resize(1, EulerTransforms());
	cocktailWaterModel_.worldMatrices_.resize(1, Matrix4x4());
}

void TitlePhase::Initialize_TitleRayModel() {
	titleRayModel_.Models_ = std::make_unique<RenderObject>();

	// SpotLight.PS.hlslにはVSMainとPSMainの両方が入っているため、
	// Vertex ShaderとPixel Shaderで同じファイルを指定する
	titleRayModel_.Models_->psoConfig_.vs = "assets/shaders/SpotLightRay/SpotLightRay.VS.hlsl";
	titleRayModel_.Models_->psoConfig_.ps = "assets/shaders/SpotLightRay/SpotLightRay.PS.hlsl";
	titleRayModel_.Models_->SetupFromShaders();

	titleRayModel_.Models_->modelID_ = titleRayModel_.ID;
	titleRayModel_.Models_->instanceNum_ = 1;
	titleRayModel_.transforms_.resize(1, EulerTransforms());
	titleRayModel_.worldMatrices_.resize(1, Matrix4x4());
}

void TitlePhase::Initialize_LightModels() {

	Initialize_Models(barModel_);
	Initialize_Models(glassModel_);
	Initialize_Models(CocktailModel_);
	Initialize_WaterModel();
	Initialize_TitleRayModel();
	Initialize_Models(ginModel_);
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		Initialize_Models(titleSelectModels_[i]);
	}
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Initialize_Models(iceModel_[i]);
	}

	glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -60.0f));
	CocktailModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	cocktailWaterModel_.transforms_[0] = EulerTransforms(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	titleRayPosition_ = Vector3(kTitleRayPositionX_, kTitleRayPositionY_, kTitleRayPositionZ_);
	titleRayModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), titleRayPosition_);
	ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -50.0f));

	// GIFのマゼンタ色の光を基準にしたTitleRay設定
	titleRayMaterialBuffer_.color = Vector4(0.74f, 0.28f, 0.39f, 0.20f);
	titleRayMaterialBuffer_.intensity = 2.5f;
	titleRayMaterialBuffer_.tipRadius = 0.4f;
	titleRayMaterialBuffer_.endRadius = 5.0f;
	titleRayMaterialBuffer_.coneLength = 16.0f;
	titleRayMaterialBuffer_.reveal = 0.0f;
	titleRayMaterialBuffer_.revealSoftness = 0.08f;
	titleRayMaterialBuffer_.density = 1.0f;
	titleRayMaterialBuffer_.centerBrightness = 1.5f;
	titleRayMaterialBuffer_.edgeSoftness = 0.25f;
	titleRayMaterialBuffer_.distanceFade = 0.35f;
	titleRayMaterialBuffer_.stepCount = 48;
	titleRayMaterialBuffer_.padding = 0.0f;

	waterWaveBuffer_.relativeScale = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
	waterWaveBuffer_.waveAxisXWS = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	waterWaveBuffer_.waveAxisYWS = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	waterWaveBuffer_.waveAxisZWS = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	waterWaveBuffer_.surfaceY = 7.0f;
	waterWaveBuffer_.commonWorldScale = 10.0f;
	waterWaveBuffer_.sideWaveDepth = 0.5f;
	waterWaveBuffer_.waveHeight = 0.02f;
	waterWaveBuffer_.waveFrequency = 2.0f;
	waterWaveBuffer_.waveSpeed = 1.0f;
	waterWaveBuffer_.motionHeightBoost = 0.35f;
	waterWaveBuffer_.motionIntensity = 0.0f;

	waterColorBuffer_.colorA = Vector4(1.00f, 0.32f, 0.55f, 1.0f);
	waterColorBuffer_.colorB = Vector4(0.10f, 0.55f, 1.00f, 1.0f);
	waterColorBuffer_.baseColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	waterColorBuffer_.colorBalance = 0.5f;
	waterColorBuffer_.colorBlendWidth = 0.25f;
	waterColorBuffer_.colorDistortion = 0.3f;
	waterColorBuffer_.colorPatternScale = 1.0f;
	waterColorBuffer_.convectionSpeed = 0.35f;
	waterColorBuffer_.convectionStrength = 0.2f;
	waterColorBuffer_.convectionScale = 1.0f;
	waterColorBuffer_.mixProgress = 1.0f;
	waterColorBuffer_.motionIntensity = 0.0f;
	waterColorBuffer_.smoothness = 0.8f;
	waterColorBuffer_.fresnelStrength = 0.2f;

	waterLightingBuffer_.mainLightDirection = Vector4(0.0f, 0.70710678f, -0.70710678f, 0.0f);
	waterLightingBuffer_.mainLightColor = Vector4(1.0f, 0.35f, 0.05f, 1.0f);
	waterLightingBuffer_.ambientSky = Vector4(0.15f, 0.15f, 0.18f, 1.0f);
	waterLightingBuffer_.ambientGround = Vector4(0.05f, 0.04f, 0.04f, 1.0f);

	// Selectになるまでは描画しないが、2つの選択モデルを先に初期化しておく
	titleSelectModels_[0].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectSelectedScale_, kTitleSelectSelectedScale_, kTitleSelectSelectedScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, 8.0f, -60.0f));
	titleSelectModels_[1].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectNormalScale_, kTitleSelectNormalScale_, kTitleSelectNormalScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, 8.0f, -50.0f));

	Initialize_IceTransforms();

	// ========================================
	// Light Buffer
	// ========================================

	lightBuffer_ = {};

	// 環境光
	lightBuffer_.ambientColor = Vector3(0.15f, 0.15f, 0.15f);

	// Directional Light + Spot Light x 6
	lightBuffer_.lightCount = 7;

	// ========================================
	// Directional Light
	// ========================================

	Light& directionalLight = lightBuffer_.lights[0];

	directionalLight.type = 0;

	directionalLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	// 全体を暗めに照らす
	directionalLight.intensity = 0.25f;

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

	// ========================================
	// Spot Lights
	// ========================================

	constexpr float spotLightZPositions[] = {
	    -60.0f, -75.0f, -55.0f, -50.0f, -35.0f, -10.0f,
	};

	for (int32_t i = 0; i < 6; ++i) {
		Light& spotLight = lightBuffer_.lights[1 + i];

		spotLight.type = 2;

		// オレンジ色
		spotLight.color = Vector4(1.0f, 0.35f, 0.05f, 1.0f);

		spotLight.intensity = 4.0f;

		// 真下へ照らす
		spotLight.direction = Vector3(0.0f, -1.0f, 0.0f);

		spotLight.position = Vector3(-60.0f, 10.0f, spotLightZPositions[i]);

		spotLight.radius = 4.0f;

		spotLight.decay = 2.0f;

		spotLight.distance = 15.0f;

		// 外側：約36.9度
		spotLight.cosAngle = 0.8f;

		// 内側：約25.8度。この範囲までは最大強度で照らす
		spotLight.cosFalloffStart = 0.9f;
	}
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
		// 1,2,3,4の位置を4,3,2,1へ反転する
		const float x = backStartX + static_cast<float>(backIceCount - 1 - i) * kIceHorizontalSpacing_;

		iceTargetPositions_[backStartIndex + i] = rotateTargetPosition(x, backZ);
	}

	// 5～9番：真ん中
	constexpr int32_t middleStartIndex = 4;
	constexpr int32_t middleIceCount = 5;
	const float middleStartX = kIceTargetCenterX_ - (static_cast<float>(middleIceCount - 1) * kIceHorizontalSpacing_ * 0.5f);

	for (int32_t i = 0; i < middleIceCount; ++i) {
		// 5,6,7,8,9の位置を9,8,7,6,5へ反転する
		const float x = middleStartX + static_cast<float>(middleIceCount - 1 - i) * kIceHorizontalSpacing_;

		iceTargetPositions_[middleStartIndex + i] = rotateTargetPosition(x, kIceTargetCenterZ_);
	}

	// 10～11番：手前側
	constexpr int32_t frontStartIndex = 9;
	constexpr int32_t frontIceCount = 2;
	const float frontStartX = kIceTargetCenterX_ - (static_cast<float>(frontIceCount - 1) * kIceHorizontalSpacing_ * 0.5f);
	const float frontZ = kIceTargetCenterZ_ + kIceDepthSpacing_;

	for (int32_t i = 0; i < frontIceCount; ++i) {
		// 10,11の位置を11,10へ反転する
		const float x = frontStartX + static_cast<float>(frontIceCount - 1 - i) * kIceHorizontalSpacing_;

		iceTargetPositions_[frontStartIndex + i] = rotateTargetPosition(x, frontZ);
	}

	entranceElapsedTime_ = 0.0f;
	isEntranceFinished_ = false;
	entranceHoldElapsedTime_ = 0.0f;
	isEntranceHoldFinished_ = false;
	titleRayRevealElapsedTime_ = 0.0f;
	isTitleRayRevealFinished_ = false;
	titleSelectRayRevealElapsedTime_ = 0.0f;
	isTitleSelectRayActive_ = false;
	titleRayPosition_ = Vector3(kTitleRayPositionX_, kTitleRayPositionY_, kTitleRayPositionZ_);
	titleRayMaterialBuffer_.reveal = 0.0f;
	preLiftElapsedTime_ = 0.0f;
	isPreLiftFinished_ = false;
	glassTiltElapsedTime_ = 0.0f;
	isGlassTiltFinished_ = false;
	glassTiltHoldElapsedTime_ = 0.0f;
	isGlassTiltHoldFinished_ = false;
	iceAnimationElapsedTime_ = 0.0f;
	isIceAnimationFinished_ = false;
	isIcePseudoPhysicsReleased_ = false;
	isIceSubmergeFinished_ = false;
	isIceFloatStarted_ = false;
	iceOverheadCameraElapsedTime_ = 0.0f;
	iceSettledHoldElapsedTime_ = 0.0f;
	iceFloatWaitElapsedTime_ = 0.0f;
	isCocktailCameraStarted_ = false;
	isSideCameraReturned_ = false;
	glassReturnElapsedTime_ = 0.0f;
	isGlassReturnFinished_ = false;
	ginAnimationElapsedTime_ = 0.0f;
	isGinCameraStarted_ = false;
	isCocktailViewCameraStarted_ = false;
	isCocktailPeekCameraStarted_ = false;
	titleSelectCameraElapsedTime_ = 0.0f;
	titleSelectTransitionElapsedTime_ = 0.0f;
	isTitleSelectTransitionStarted_ = false;
	isTitleSelectInputEnabled_ = false;
	titleSelectPulseElapsedTime_ = 0.0f;
	cocktailWaterScaleElapsedTime_ = 0.0f;
	cocktailWaterAnimationTime_ = 0.0f;
	isCocktailWaterAppearing_ = false;
	titlePhaseSelection_ = TitlePhaseSelection::Start;
	selectedTitleIndex_ = 0;
	isSelectionConfirmed_ = false;
	selectionConfirmElapsedTime_ = 0.0f;
	selectionCameraElapsedTime_ = 0.0f;
	selectionCameraTheta_ = 0.0f;
	selectedCocktailTargetZ_ = kIceTargetCenterZ_;
	presentationCameraPosition_ = Vector3(0.0f, 0.0f, 0.0f);
	isPresentationCameraPositionFixed_ = false;
	glassCameraStartFocus_ = Vector3(0.0f, 0.0f, 0.0f);
	iceTransferCameraStartFocus_ = Vector3(0.0f, 0.0f, 0.0f);
	ginCameraStartFocus_ = Vector3(0.0f, 0.0f, 0.0f);

	// 最初は、グラスと氷を-Z側、ジンを+Z側へ離しておく
	const Vector3 glassEntrancePosition(-60.0f, 7.0f, -60.0f - kEntranceZDistance_);
	glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), glassEntrancePosition);

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		const Vector3 iceEntrancePosition = iceStartPositions_[i] + Vector3(0.0f, 0.0f, -kEntranceZDistance_);
		iceModel_[i].transforms_[0] = EulerTransforms(Vector3(0.2f, 0.2f, 0.2f), Vector3(0.0f, 0.0f, 0.0f), iceEntrancePosition);
	}

	const Vector3 ginEntrancePosition(-60.0f, 7.0f, -50.0f + kEntranceZDistance_);
	ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), ginEntrancePosition);

	// TitleSelectは切り替え演出が始まるまで完成位置の下で待機させる
	titleSelectModels_[0].transforms_[0] = EulerTransforms(
	    Vector3(kTitleSelectSelectedScale_, kTitleSelectSelectedScale_, kTitleSelectSelectedScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, 8.0f - kTitleSelectRiseDistance_, -60.0f));
	titleSelectModels_[1].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectNormalScale_, kTitleSelectNormalScale_, kTitleSelectNormalScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, 8.0f - kTitleSelectRiseDistance_, -50.0f));

	Initialize_PresentationIce();

	previousAnimationTime_ = std::chrono::steady_clock::now();
	previousSelectionAnimationTime_ = previousAnimationTime_;
}

void TitlePhase::Initialize_PresentationIce() {
	sourceGlassIcePosition_ = Vector3(-60.0f, 7.0f, -60.0f - kEntranceZDistance_);
	sourceGlassIceRotationX_ = 0.0f;
	const float goldenAngle = std::numbers::pi_v<float> * (3.0f - std::sqrt(5.0f));

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		IcePresentationState& ice = presentationIce_[i];
		ice = IcePresentationState{};
		ice.position = iceStartPositions_[i] + Vector3(0.0f, 0.0f, -kEntranceZDistance_);
		ice.rotation = Vector3(0.0f, 0.0f, 0.0f);
		ice.scale = Vector3(0.2f, 0.2f, 0.2f);
		ice.sourceLocalPosition = Vector3(ice.position.x - sourceGlassIcePosition_.x, ice.position.y - sourceGlassIcePosition_.y, ice.position.z - sourceGlassIcePosition_.z);
		ice.releaseDelay = kIceFirstReleaseDelay_ + kIceMoveStartDelay_ + static_cast<float>(i) * kIceReleaseInterval_;
		// 0,7,3,10...のように番号順ではない順番で浮上させる。
		ice.floatDelay = static_cast<float>((i * 7) % kMaxIceCount_) * kIceFloatInterval_;
		ice.floatDurationScale = 0.85f + static_cast<float>((i * 5) % 4) * 0.10f;
		// 同じ軌道に重ならないよう、速度とX方向の初速を氷ごとに変える。
		ice.driveSpeedScale = 0.88f + static_cast<float>((i * 7) % 5) * 0.04f;
		ice.horizontalDrift = static_cast<float>((i % 3) - 1) * 0.35f;

		// 最終整列位置とは無関係な、円錐内部の乱雑な沈下位置を氷ごとに作る。
		const float scatteredValue = std::fmod(0.31f + static_cast<float>(i) * 0.6180339887f, 1.0f);
		const float scatteredRadius = kIceSubmergedScatterRadius_ * (0.2f + 0.8f * std::sqrt(scatteredValue));
		const float scatteredAngle = goldenAngle * static_cast<float>(i) + static_cast<float>(i % 4) * 0.21f;
		const float submergedY = kIceTargetHeightY_ - kIceSubmergedDepth_ - static_cast<float>(i % 4) * kIceSubmergedDepthVariation_;
		ice.submergedPosition = Vector3(kIceTargetCenterX_ + std::cos(scatteredAngle) * scatteredRadius, submergedY, kIceTargetCenterZ_ + std::sin(scatteredAngle) * scatteredRadius);
		ice.submergedRotation = Vector3(
		    (static_cast<float>((i * 5) % 9) - 4.0f) * std::numbers::pi_v<float> / 9.0f, (static_cast<float>((i * 7) % 11) - 5.0f) * std::numbers::pi_v<float> / 10.0f,
		    (static_cast<float>((i * 3) % 7) - 3.0f) * std::numbers::pi_v<float> / 8.0f);
	}

	Sync_IceModelsFromPresentation();
}

float TitlePhase::Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

float TitlePhase::EaseInOut01(float value) {
	// 0～1の補間率を、開始時と終了時が滑らかな3次イーズインアウトへ変換する
	const float t = Clamp01(value);
	return t * t * (3.0f - 2.0f * t);
}

float TitlePhase::VectorLength(const Vector3& value) { return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z); }

Vector3 TitlePhase::NormalizeVector(const Vector3& value) {
	const float length = VectorLength(value);
	if (length <= 0.000001f) {
		return Vector3(0.0f, 0.0f, 0.0f);
	}
	return value * (1.0f / length);
}

void TitlePhase::Update_IceSourceTransform(const Vector3& position, float rotationX) {
	sourceGlassIcePosition_ = position;
	sourceGlassIceRotationX_ = rotationX;
}

void TitlePhase::Update_PresentationIce(float deltaTime, bool updatePhysics) {
	// Release前は元グラスのローカル位置を保ったまま移動・回転へ追従する。
	const float rotationCos = std::cos(sourceGlassIceRotationX_);
	const float rotationSin = std::sin(sourceGlassIceRotationX_);
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		IcePresentationState& ice = presentationIce_[i];
		if (ice.isFollowingSourceGlass && !ice.hasReleased) {
			const Vector3& local = ice.sourceLocalPosition;
			ice.position = Vector3(
			    sourceGlassIcePosition_.x + local.x, sourceGlassIcePosition_.y + local.y * rotationCos - local.z * rotationSin,
			    sourceGlassIcePosition_.z + local.y * rotationSin + local.z * rotationCos);
			ice.rotation = Vector3(sourceGlassIceRotationX_, 0.0f, 0.0f);
		}
	}

	if (updatePhysics) {
		Update_PresentationIcePhysics(deltaTime);
	} else {
		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			if (presentationIce_[i].hasReachedTarget) {
				Update_IceWave(presentationIce_[i], i);
			}
		}
	}

	Sync_IceModelsFromPresentation();
}

void TitlePhase::Update_PresentationIcePhysics(float deltaTime) {
	const Vector3 cocktailCenter(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		IcePresentationState& ice = presentationIce_[i];
		if (!ice.isPhysicsActive || ice.isFloating || ice.hasReachedTarget) {
			continue;
		}

		if (!ice.hasEnteredCocktailGlass) {
			const Vector3 driveDirection = NormalizeVector(Vector3(cocktailCenter.x - ice.position.x, cocktailCenter.y - ice.position.y, cocktailCenter.z - ice.position.z));
			ice.velocity = ice.velocity + driveDirection * (kIceDriveAcceleration_ * deltaTime);
			const float speed = VectorLength(ice.velocity);
			const float maximumSpeed = kIceMaxDriveSpeed_ * ice.driveSpeedScale;
			if (speed > maximumSpeed) {
				ice.velocity = ice.velocity * (maximumSpeed / speed);
			}
		}

		ice.velocity.y -= kIceGravity_ * deltaTime;
		ice.position = ice.position + ice.velocity * deltaTime;

		const Vector3 offset(ice.position.x - cocktailCenter.x, ice.position.y - cocktailCenter.y, ice.position.z - cocktailCenter.z);
		const float horizontalDistanceSq = offset.x * offset.x + offset.z * offset.z;
		if (!ice.hasEnteredCocktailGlass && horizontalDistanceSq <= kCocktailCollisionRadius_ * kCocktailCollisionRadius_ && offset.y >= kCocktailCollisionBottomLocalY_ &&
		    offset.y <= kCocktailCollisionTopLocalY_) {
			ice.hasEnteredCocktailGlass = true;
			ice.isPhysicsActive = false;
			ice.velocity = Vector3(0.0f, 0.0f, 0.0f);
			ice.sinkStartPosition = ice.position;
			ice.floatStartRotation = ice.rotation;
			ice.sinkTimer = 0.0f;
		}

		if (ice.hasEnteredCocktailGlass) {
			Apply_CocktailGlassCollision(ice);
		}

		// 接線速度に応じた見た目用の転がり。
		ice.rotation.x += ice.velocity.z * kIceRollMultiplier_ * deltaTime;
		ice.rotation.z -= ice.velocity.x * kIceRollMultiplier_ * deltaTime;
	}

	Apply_IceSeparation(deltaTime);

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Update_IceFloat(presentationIce_[i], i, deltaTime);
	}
}

void TitlePhase::Apply_CocktailGlassCollision(IcePresentationState& ice) {
	const Vector3 center(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);
	const float bottomY = center.y + kCocktailCollisionBottomLocalY_ + kIceCollisionHalfExtent_;
	const float safeRadius = kCocktailCollisionRadius_ - kIceCollisionHalfExtent_;

	// カクテルグラスの底面との衝突。
	if (ice.position.y < bottomY) {
		ice.position.y = bottomY;
		if (ice.velocity.y < 0.0f) {
			ice.velocity.y = -ice.velocity.y * kIceBounce_;
			ice.velocity.x *= kIceSlideRetention_;
			ice.velocity.z *= kIceSlideRetention_;
		}
	}

	// カクテルグラスの内壁との衝突。上側は開いているためYは制限しない。
	float offsetX = ice.position.x - center.x;
	float offsetZ = ice.position.z - center.z;
	const float horizontalDistance = std::sqrt(offsetX * offsetX + offsetZ * offsetZ);
	if (horizontalDistance > safeRadius && horizontalDistance > 0.000001f) {
		const float outwardX = offsetX / horizontalDistance;
		const float outwardZ = offsetZ / horizontalDistance;
		ice.position.x = center.x + outwardX * safeRadius;
		ice.position.z = center.z + outwardZ * safeRadius;

		const float outwardSpeed = ice.velocity.x * outwardX + ice.velocity.z * outwardZ;
		if (outwardSpeed > 0.0f) {
			ice.velocity.x -= outwardX * outwardSpeed * (1.0f + kIceBounce_);
			ice.velocity.z -= outwardZ * outwardSpeed * (1.0f + kIceBounce_);
		}
	}
}

void TitlePhase::Apply_IceSeparation(float deltaTime) {
	const float minimumDistance = kIceCollisionHalfExtent_ * 2.0f;
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		IcePresentationState& first = presentationIce_[i];
		if (!first.hasEnteredCocktailGlass || !first.isPhysicsActive) {
			continue;
		}

		for (int32_t j = i + 1; j < kMaxIceCount_; ++j) {
			IcePresentationState& second = presentationIce_[j];
			if (!second.hasEnteredCocktailGlass || !second.isPhysicsActive) {
				continue;
			}

			Vector3 difference(first.position.x - second.position.x, 0.0f, first.position.z - second.position.z);
			float distance = VectorLength(difference);
			if (distance >= minimumDistance) {
				continue;
			}
			if (distance <= 0.000001f) {
				difference = Vector3(i % 2 == 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
				distance = 1.0f;
			}

			const Vector3 direction = difference * (1.0f / distance);
			const float pushSpeed = std::min(kIceSeparationAcceleration_ * (minimumDistance - distance) * deltaTime, kIceMaximumSeparationSpeed_ * deltaTime);
			first.position = first.position + direction * pushSpeed;
			second.position = second.position + direction * (-pushSpeed);
			Apply_CocktailGlassCollision(first);
			Apply_CocktailGlassCollision(second);
		}
	}
}

void TitlePhase::Update_IceFloat(IcePresentationState& ice, int32_t iceIndex, float deltaTime) {
	if (!ice.hasEnteredCocktailGlass || ice.hasReachedTarget) {
		if (ice.hasReachedTarget) {
			Update_IceWave(ice, iceIndex);
		}
		return;
	}

	// まず円錐状のカクテルグラス内へ沈め、面配置の真下で待機させる。
	if (!ice.hasSunkInCocktailGlass) {
		ice.sinkTimer += deltaTime;
		const float sinkT = EaseInOut01(ice.sinkTimer / kIceSinkDuration_);
		ice.position = ice.sinkStartPosition * (1.0f - sinkT) + ice.submergedPosition * sinkT;
		ice.rotation = ice.floatStartRotation * (1.0f - sinkT) + ice.submergedRotation * sinkT;
		if (ice.sinkTimer >= kIceSinkDuration_) {
			ice.position = ice.submergedPosition;
			ice.rotation = ice.submergedRotation;
			ice.hasSunkInCocktailGlass = true;
		}
		return;
	}

	// 全氷が沈み、ジンの傾斜演出が終わるまでは浮上させない。
	if (!isIceFloatStarted_) {
		return;
	}

	const Vector3 targetRotation(0.0f, -std::numbers::pi_v<float> * 0.5f, 0.0f);
	const Vector3 arrangedUnderwaterPosition(iceTargetPositions_[iceIndex].x, kIceTargetHeightY_ - kIceSubmergedDepth_, iceTargetPositions_[iceIndex].z);

	// 全ての氷を、まず水面下で最終配置のX・Zへ並べる。
	if (!ice.hasArranged) {
		if (!ice.isArranging) {
			ice.isArranging = true;
			ice.arrangeTimer = 0.0f;
			ice.arrangeStartPosition = ice.position;
			ice.arrangeStartRotation = ice.rotation;
		}

		ice.arrangeTimer += deltaTime;
		const float arrangeT = EaseInOut01(ice.arrangeTimer / kIceArrangeDuration_);
		ice.position = ice.arrangeStartPosition * (1.0f - arrangeT) + arrangedUnderwaterPosition * arrangeT;
		ice.rotation = ice.arrangeStartRotation * (1.0f - arrangeT) + targetRotation * arrangeT;

		if (ice.arrangeTimer >= kIceArrangeDuration_) {
			ice.position = arrangedUnderwaterPosition;
			ice.rotation = targetRotation;
			ice.isArranging = false;
			ice.hasArranged = true;
			ice.floatTimer = 0.0f;
		}
		return;
	}

	// 全氷の整列後、氷ごとの待機時間と速度でバラバラに浮上させる。
	ice.floatTimer += deltaTime;
	if (ice.floatTimer < ice.floatDelay) {
		return;
	}

	if (!ice.isFloating) {
		ice.isFloating = true;
		ice.floatStartPosition = ice.position;
		ice.floatStartRotation = ice.rotation;
	}

	const float activeFloatTime = ice.floatTimer - ice.floatDelay;
	const float iceFloatDuration = kIceFloatDuration_ * ice.floatDurationScale;
	const float floatT = EaseInOut01(activeFloatTime / iceFloatDuration);
	ice.position = ice.floatStartPosition * (1.0f - floatT) + iceTargetPositions_[iceIndex] * floatT;
	ice.rotation = ice.floatStartRotation * (1.0f - floatT) + targetRotation * floatT;

	if (activeFloatTime >= iceFloatDuration) {
		ice.position = iceTargetPositions_[iceIndex];
		ice.rotation = targetRotation;
		ice.isFloating = false;
		ice.hasReachedTarget = true;
	}
}

void TitlePhase::Update_IceWave(IcePresentationState& ice, int32_t iceIndex) {
	const Vector3 target = iceTargetPositions_[iceIndex];
	const float objectX = (target.x - kIceTargetCenterX_) / std::max(waterWaveBuffer_.commonWorldScale, 0.0001f);
	const float objectZ = (target.z - kIceTargetCenterZ_) / std::max(waterWaveBuffer_.commonWorldScale, 0.0001f);
	const float phaseX = objectX * waterWaveBuffer_.waveFrequency + cocktailWaterAnimationTime_ * waterWaveBuffer_.waveSpeed;
	const float phaseZ = objectZ * waterWaveBuffer_.waveFrequency * 0.83f - cocktailWaterAnimationTime_ * waterWaveBuffer_.waveSpeed * 0.71f;
	const float heightMultiplier = 1.0f + waterWaveBuffer_.motionIntensity * waterWaveBuffer_.motionHeightBoost;
	const float wave = std::sin(phaseX) * std::cos(phaseZ) * waterWaveBuffer_.waveHeight * heightMultiplier * waterWaveBuffer_.commonWorldScale;
	ice.position = target + Vector3(0.0f, wave, 0.0f);
	ice.rotation = Vector3(0.0f, -std::numbers::pi_v<float> * 0.5f, 0.0f);
}

void TitlePhase::Sync_IceModelsFromPresentation() {
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		const IcePresentationState& ice = presentationIce_[i];
		iceModel_[i].transforms_[0] = EulerTransforms(ice.scale, ice.rotation, ice.position);
	}
}

void TitlePhase::AimCameraFromFixedPosition(const Vector3& cameraPosition, const Vector3& target) {
	const float directionX = target.x - cameraPosition.x;
	const float directionY = target.y - cameraPosition.y;
	const float directionZ = target.z - cameraPosition.z;
	const float distance = std::sqrt(directionX * directionX + directionY * directionY + directionZ * directionZ);

	if (distance <= 0.0001f) {
		return;
	}

	// Orbitカメラの角度規則へ合わせ、固定位置から対象を見るPhi・Thetaを求める
	float normalizedY = directionY / distance;
	if (normalizedY < -1.0f) {
		normalizedY = -1.0f;
	} else if (normalizedY > 1.0f) {
		normalizedY = 1.0f;
	}

	const float phi = std::asin(-normalizedY);
	const float theta = std::atan2(-directionZ, -directionX);

	// Center変更による位置変化を、同時に設定する角度と距離で打ち消す
	Game::Camera::Setter::SetCenter(target, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(phi, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(theta, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetDistance(distance, 0.0f, EaseType::IN_OUT_SINE, c_main_);
}

void TitlePhase::Update_Animation() {
	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	float deltaTime = std::chrono::duration<float>(currentTime - previousAnimationTime_).count();

	previousAnimationTime_ = currentTime;

	// デバッグ停止などで極端に大きな時間が入った場合の瞬間移動を防ぐ
	if (deltaTime > 0.1f) {
		deltaTime = 0.1f;
	}

	// 液体が表示された後は、Select中も波と対流を動かし続ける
	if (isCocktailWaterAppearing_) {
		cocktailWaterAnimationTime_ += deltaTime;
	}

	// 完成位置へ浮いた後も、選択確定までは水面の波へ追従させる。
	if (isIceAnimationFinished_ && !isSelectionConfirmed_) {
		Update_PresentationIce(deltaTime, false);
	}

	// ジンのアニメーションまで完了した後は、水面時間だけ更新して終了
	if (titlePhaseSelection_ == TitlePhaseSelection::Select) {
		titleSelectPulseElapsedTime_ += deltaTime;

		// 選択中のTitleRayを、選択位置の上から下へ滑らかに伸ばす。
		if (isTitleSelectRayActive_) {
			titleSelectRayRevealElapsedTime_ += deltaTime;
			const float rayT = Clamp01(titleSelectRayRevealElapsedTime_ / kTitleRayRevealDuration_);
			titleRayMaterialBuffer_.reveal = EaseInOut01(rayT);
		}
		return;
	}

	// ========================================
	// TitleRay Reveal Animation
	// ========================================

	if (!isTitleRayRevealFinished_) {
		titleRayRevealElapsedTime_ += deltaTime;

		float revealT = titleRayRevealElapsedTime_ / kTitleRayRevealDuration_;
		if (revealT >= 1.0f) {
			revealT = 1.0f;
			isTitleRayRevealFinished_ = true;
		}

		// 上から下へ光が滑らかに伸び切ってから、左右の登場を始める
		titleRayMaterialBuffer_.reveal = EaseInOut01(revealT);
		return;
	}

	// ========================================
	// Glass, Ice And Gin Entrance Animation
	// ========================================

	if (!isEntranceFinished_) {
		entranceElapsedTime_ += deltaTime;

		float entranceT = entranceElapsedTime_ / kEntranceDuration_;
		if (entranceT >= 1.0f) {
			entranceT = 1.0f;
			isEntranceFinished_ = true;
		}

		// 始点と終点で速度が0になる滑らかな補間
		const float smoothEntranceT = EaseInOut01(entranceT);

		const float glassZ = (-60.0f - kEntranceZDistance_) * (1.0f - smoothEntranceT) + (-60.0f) * smoothEntranceT;
		glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, glassZ));
		Update_IceSourceTransform(Vector3(-60.0f, 7.0f, glassZ), 0.0f);
		Update_PresentationIce(deltaTime, false);

		const float ginZ = (-50.0f + kEntranceZDistance_) * (1.0f - smoothEntranceT) + (-50.0f) * smoothEntranceT;
		ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, ginZ));

		// 全員が定位置へ到着してから持ち上げ演出へ進む
		return;
	}

	// ========================================
	// Entrance Hold
	// ========================================

	// 映像と同じく、3つのモデルが揃った正面画をしばらく見せてから
	// カメラを氷用グラスへ向け始める。
	if (!isEntranceHoldFinished_) {
		entranceHoldElapsedTime_ += deltaTime;
		if (entranceHoldElapsedTime_ >= kEntranceHoldDuration_) {
			entranceHoldElapsedTime_ = kEntranceHoldDuration_;
			isEntranceHoldFinished_ = true;
		}
		return;
	}

	// ========================================
	// Glass And Ice Pre-Lift Animation
	// ========================================

	if (!isPreLiftFinished_) {
		if (!isPresentationCameraPositionFixed_) {
			isPresentationCameraPositionFixed_ = true;
			presentationCameraPosition_ = Game::Camera::Getter::GetWorldPosition(c_main_);
			glassCameraStartFocus_ = Game::Camera::Getter::GetCenter(c_main_);
		}

		preLiftElapsedTime_ += deltaTime;

		float preparationT = preLiftElapsedTime_ / kPreLiftDuration_;
		if (preparationT >= 1.0f) {
			preparationT = 1.0f;
			isPreLiftFinished_ = true;
		}

		// この段階ではグラスを持ち上げない。
		// カメラの準備が終わった次の段階で、持ち上げと傾斜を同時に行う。
		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(0.0f, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f, -60.0f);
		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);
		Update_IceSourceTransform(glassPosition, 0.0f);
		Update_PresentationIce(deltaTime, false);

		// 氷用グラスへ向きを変えるのと同時に、カメラZは固定したまま
		// X・Yをカクテルグラス側の撮影位置へ滑らかに近づける。
		float glassCameraT = preLiftElapsedTime_ / kGlassCameraLookDuration_;
		if (glassCameraT > 1.0f) {
			glassCameraT = 1.0f;
		}
		const float smoothGlassCameraT = EaseInOut01(glassCameraT);
		const Vector3 glassCameraFocus = glassCameraStartFocus_ * (1.0f - smoothGlassCameraT) + glassPosition * smoothGlassCameraT;
		const Vector3 cocktailCameraTarget(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);
		const float targetHorizontalDistance = kIceTransferCameraDistance_ * std::cos(kIceTransferCameraPhi_);
		const Vector3 cocktailCameraEndPosition(
		    cocktailCameraTarget.x + targetHorizontalDistance * std::cos(kIceTransferCameraTheta_), cocktailCameraTarget.y + kIceTransferCameraDistance_ * std::sin(kIceTransferCameraPhi_),
		    presentationCameraPosition_.z);
		const Vector3 currentGlassCameraPosition(
		    presentationCameraPosition_.x * (1.0f - smoothGlassCameraT) + cocktailCameraEndPosition.x * smoothGlassCameraT,
		    presentationCameraPosition_.y * (1.0f - smoothGlassCameraT) + cocktailCameraEndPosition.y * smoothGlassCameraT, presentationCameraPosition_.z);
		AimCameraFromFixedPosition(currentGlassCameraPosition, glassCameraFocus);

		// カメラの準備が終わってから、グラスの持ち上げと傾斜を開始する
		return;
	}

	// ========================================
	// Glass Tilt And Ice Move Animation
	// ========================================

	if (!isGlassTiltHoldFinished_) {
		// 氷が沈み切った後は、投入中の傾斜姿勢でグラスを再設定しない。
		// 復帰後に空中の傾斜姿勢へ戻って固定される上書きを防ぐ。
		if (!isIceSubmergeFinished_) {
			// グラスの傾斜と氷の移動開始に合わせて、カメラZは固定したまま
			// X・Yと角度をカクテルグラスの斜め上構図へ変える。
			if (!isCocktailCameraStarted_) {
				isCocktailCameraStarted_ = true;
				Start_CocktailCameraAnimation();
			}

			iceAnimationElapsedTime_ += deltaTime;
			glassTiltElapsedTime_ += deltaTime;

			// 最初の氷を放すまでに、カメラZを固定したまま
			// カクテルグラスを少し上から覗く位置と角度へ補間する。
			// カメラの補間完了後、さらにkIceMoveStartDelay_秒停止してから
			// 最初の氷を移動させる。
			const float iceCameraT = EaseInOut01(iceAnimationElapsedTime_ / kIceTransferCameraDuration_);
			const Vector3 cocktailCameraTarget(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);
			const float targetHorizontalDistance = kIceTransferCameraDistance_ * std::cos(kIceTransferCameraPhi_);
			const Vector3 cocktailCameraEndPosition(
			    cocktailCameraTarget.x + targetHorizontalDistance * std::cos(kIceTransferCameraTheta_), cocktailCameraTarget.y + kIceTransferCameraDistance_ * std::sin(kIceTransferCameraPhi_),
			    presentationCameraPosition_.z);
			const Vector3 currentIceCameraPosition(
			    presentationCameraPosition_.x * (1.0f - iceCameraT) + cocktailCameraEndPosition.x * iceCameraT,
			    presentationCameraPosition_.y * (1.0f - iceCameraT) + cocktailCameraEndPosition.y * iceCameraT, presentationCameraPosition_.z);
			const Vector3 currentIceCameraFocus = iceTransferCameraStartFocus_ * (1.0f - iceCameraT) + cocktailCameraTarget * iceCameraT;
			AimCameraFromFixedPosition(currentIceCameraPosition, currentIceCameraFocus);

			float tiltT = glassTiltElapsedTime_ / kGlassTiltDuration_;
			if (tiltT >= 1.0f) {
				tiltT = 1.0f;
				isGlassTiltFinished_ = true;
			}
			const float smoothTiltT = EaseInOut01(tiltT);
			const float currentTiltAngle = kGlassTiltAngle_ * smoothTiltT;
			const float currentLiftHeight = kGlassLiftHeight_ * smoothTiltT;
			const float currentMoveZ = kGlassTiltMoveZ_ * smoothTiltT;

			const Vector3 glassScale(10.0f, 10.0f, 10.0f);
			const Vector3 glassRotate(currentTiltAngle, 0.0f, 0.0f);
			const Vector3 glassPosition(-60.0f, 7.0f + currentLiftHeight, -60.0f + currentMoveZ);

			glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);
			Update_IceSourceTransform(glassPosition, currentTiltAngle);

			if (!isIcePseudoPhysicsReleased_) {
				// Release時刻を氷ごとにずらし、1個ずつ別々の軌道で移動させる。
				Update_PresentationIce(0.0f, false);
				const Vector3 cocktailCenter(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);
				bool allIceReleased = true;
				for (int32_t i = 0; i < kMaxIceCount_; ++i) {
					IcePresentationState& ice = presentationIce_[i];
					if (ice.hasReleased) {
						continue;
					}
					if (iceAnimationElapsedTime_ < ice.releaseDelay) {
						allIceReleased = false;
						continue;
					}
					ice.isFollowingSourceGlass = false;
					ice.hasReleased = true;
					ice.isPhysicsActive = true;
					ice.velocity = NormalizeVector(Vector3(cocktailCenter.x - ice.position.x, cocktailCenter.y - ice.position.y, cocktailCenter.z - ice.position.z)) * kIceMaxDriveSpeed_;
					ice.velocity.x += ice.horizontalDrift;
				}
				// このフレームで最後の氷を放した場合も含めて再確認する。
				for (int32_t i = 0; i < kMaxIceCount_; ++i) {
					if (!presentationIce_[i].hasReleased) {
						allIceReleased = false;
						break;
					}
				}
				isIcePseudoPhysicsReleased_ = allIceReleased;
			}

			Update_PresentationIce(deltaTime, true);

			isIceSubmergeFinished_ = true;
			for (int32_t i = 0; i < kMaxIceCount_; ++i) {
				if (!presentationIce_[i].hasSunkInCocktailGlass) {
					isIceSubmergeFinished_ = false;
					break;
				}
			}

			if (!isIceSubmergeFinished_) {
				return;
			}
		}

		// 氷が入り切った直後は俯瞰を保ち、その間に投入用グラスだけを
		// 元の位置・角度へ戻す。映像のようにカメラが戻る前にグラスを片付ける。
		if (!isGlassReturnFinished_) {
			glassReturnElapsedTime_ += deltaTime;
			float returnT = Clamp01(glassReturnElapsedTime_ / kGlassReturnDuration_);
			const float smoothReturnT = EaseInOut01(returnT);
			const float remainingAmount = 1.0f - smoothReturnT;

			glassModel_.transforms_[0] = EulerTransforms(
			    Vector3(10.0f, 10.0f, 10.0f), Vector3(kGlassTiltAngle_ * remainingAmount, 0.0f, 0.0f),
			    Vector3(-60.0f, 7.0f + kGlassLiftHeight_ * remainingAmount, -60.0f + kGlassTiltMoveZ_ * remainingAmount));

			if (returnT >= 1.0f) {
				isGlassReturnFinished_ = true;
			}
		}

		iceSettledHoldElapsedTime_ += deltaTime;
		if (iceSettledHoldElapsedTime_ < kIceSettledHoldDuration_) {
			return;
		}

		// 俯瞰の停止後、ZとYを固定してXのプラス方向へ引きながらジンを見る。
		if (!isGinCameraStarted_) {
			isGinCameraStarted_ = true;
			iceOverheadCameraElapsedTime_ = 0.0f;
			Start_GinCameraAnimation();
		}

		iceOverheadCameraElapsedTime_ += deltaTime;

		float ginCameraT = iceOverheadCameraElapsedTime_ / kGinViewCameraDuration_;
		if (ginCameraT > 1.0f) {
			ginCameraT = 1.0f;
		}
		const float smoothGinCameraT = EaseInOut01(ginCameraT);
		const Vector3 ginFocus(-60.0f, 7.0f, -50.0f);
		const Vector3 currentGinCameraPosition(presentationCameraPosition_.x + kGinCameraMoveX_ * smoothGinCameraT, presentationCameraPosition_.y, presentationCameraPosition_.z);
		const Vector3 currentGinCameraFocus = ginCameraStartFocus_ * (1.0f - smoothGinCameraT) + ginFocus * smoothGinCameraT;
		AimCameraFromFixedPosition(currentGinCameraPosition, currentGinCameraFocus);

		// カメラが引き終わった位置で0.5秒停止してからジンを動かす。
		const float ginAnimationStartTime = kGinViewCameraDuration_ + kGinCameraHoldDuration_;
		if (iceOverheadCameraElapsedTime_ < ginAnimationStartTime) {
			return;
		}

		presentationCameraPosition_ = Game::Camera::Getter::GetWorldPosition(c_main_);
		isGlassTiltHoldFinished_ = true;
	}

	// ========================================
	// Glass Return Animation
	// ========================================

	if (!isGlassReturnFinished_) {
		glassReturnElapsedTime_ += deltaTime;

		float returnT = glassReturnElapsedTime_ / kGlassReturnDuration_;
		if (returnT >= 1.0f) {
			returnT = 1.0f;
			isGlassReturnFinished_ = true;
		}

		// 始点と終点で速度が滑らかになるように補間する
		const float smoothReturnT = EaseInOut01(returnT);
		const float remainingAmount = 1.0f - smoothReturnT;

		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(kGlassTiltAngle_ * remainingAmount, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f + kGlassLiftHeight_ * remainingAmount, -60.0f + kGlassTiltMoveZ_ * remainingAmount);

		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);

		// グラスが元の位置と角度へ戻ってからジンのアニメーションへ進む
		if (!isGlassReturnFinished_) {
			return;
		}
	}

	// ========================================
	// Gin Animation
	// ========================================

	const Vector3 ginScale(10.0f, 10.0f, 10.0f);

	// ジンが傾き始めるのと同時に、カメラの注視点をカクテルへ戻す。
	// ジンの傾斜完了までに、Y軸角度と位置を正面へ戻しながら
	// 真上から30度傾いた角度へ補間する。
	if (!isCocktailViewCameraStarted_) {
		isCocktailViewCameraStarted_ = true;
		Start_CocktailViewCameraAnimation();
	}

	ginAnimationElapsedTime_ += deltaTime;

	const float ginHoldEndTime = kGinTiltDuration_ + kGinTiltHoldDuration_;
	const float ginAnimationEndTime = ginHoldEndTime + kGinReturnDuration_;
	float ginTiltAmount = 0.0f;

	if (ginAnimationElapsedTime_ < kGinTiltDuration_) {
		// 元の姿勢から最大角度まで滑らかに傾ける
		float tiltT = ginAnimationElapsedTime_ / kGinTiltDuration_;
		if (tiltT > 1.0f) {
			tiltT = 1.0f;
		}
		ginTiltAmount = EaseInOut01(tiltT);

	} else if (ginAnimationElapsedTime_ < ginHoldEndTime) {
		// 最大まで傾き、持ち上がった姿勢を維持する
		ginTiltAmount = 1.0f;

	} else if (ginAnimationElapsedTime_ < ginAnimationEndTime) {
		// 停止後、角度と高さを同じ補間値で元へ戻す
		float returnT = (ginAnimationElapsedTime_ - ginHoldEndTime) / kGinReturnDuration_;
		if (returnT > 1.0f) {
			returnT = 1.0f;
		}
		const float smoothReturnT = EaseInOut01(returnT);
		ginTiltAmount = 1.0f - smoothReturnT;
	}

	const Vector3 ginRotate(-std::numbers::pi_v<float> * 2 / 3.0f * ginTiltAmount, 0.0f, 0.0f);
	const Vector3 ginPosition(-60.0f, 7.0f + kGinLiftHeight_ * ginTiltAmount, -50.0f);

	ginModel_.transforms_[0] = EulerTransforms(ginScale, ginRotate, ginPosition);

	// ジンが最大まで傾いた時点から液体を表示する
	if (ginAnimationElapsedTime_ >= kGinTiltDuration_) {
		isCocktailWaterAppearing_ = true;
		cocktailWaterScaleElapsedTime_ += deltaTime;

		float waterScaleT = cocktailWaterScaleElapsedTime_ / kCocktailWaterScaleDuration_;
		if (waterScaleT > 1.0f) {
			waterScaleT = 1.0f;
		}

		const float smoothWaterScaleT = EaseInOut01(waterScaleT);
		const float waterScale = 10.0f * smoothWaterScaleT;

		cocktailWaterModel_.transforms_[0] = EulerTransforms(Vector3(waterScale, waterScale, waterScale), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	}

	// ジンが傾きと高さの両方を元へ戻し、置き終わるまでは
	// カメラをカクテルへ近づけない。
	if (ginAnimationElapsedTime_ < ginAnimationEndTime) {
		return;
	}

	// ジンを置いた後は、ジンの傾斜中に決めた角度を維持したまま
	// カクテルグラスへ距離だけ近づける。
	if (!isCocktailPeekCameraStarted_) {
		isCocktailPeekCameraStarted_ = true;
		Start_CocktailPeekCameraAnimation();
	}

	// カクテルへ近づいている間は液面だけを見せ、接近完了後も
	// 残り時間を待ってから氷の整列・浮上へ進む。
	iceFloatWaitElapsedTime_ += deltaTime;
	if (iceFloatWaitElapsedTime_ < kIceFloatWaitDuration_) {
		return;
	}

	// 待機後、円錐内の氷を「水面下で一斉に整列 → 番号順ではなく
	// バラバラの順で短く浮上」の順で動かす。
	if (!isIceFloatStarted_) {
		isIceFloatStarted_ = true;
		// 全氷の整列開始位置を同じフレームで確定する。
		Update_PresentationIce(0.0f, true);
	}

	if (!isIceAnimationFinished_) {
		Update_PresentationIce(deltaTime, true);

		isIceAnimationFinished_ = true;
		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			if (!presentationIce_[i].hasReachedTarget) {
				isIceAnimationFinished_ = false;
				break;
			}
		}
	}

	// ジンの復帰と全ての氷の浮上が終わるまで次へ進めない。
	if (ginAnimationElapsedTime_ < ginAnimationEndTime || !isIceAnimationFinished_) {
		return;
	}

	// カクテルを上寄りの斜めから覗く視点で、Spaceキー入力を待つ。
	if (!isTitleSelectTransitionStarted_) {
		if (!Game::IO::Key::IsJustPressed(0x20)) {
			return;
		}

		isTitleSelectTransitionStarted_ = true;
		// Selectへ入るまでは、最初のカクテル上のTitleRayを消しておく。
		titleRayMaterialBuffer_.reveal = 0.0f;
		titleSelectCameraElapsedTime_ = 0.0f;
		titleSelectTransitionElapsedTime_ = 0.0f;

		// 先にカメラだけを最初の状態へ戻す
		Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), kTitleSelectTransitionDuration_, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, kTitleSelectTransitionDuration_, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetThetaTarget(0.0f, kTitleSelectTransitionDuration_, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetDistance(20.0f, kTitleSelectTransitionDuration_, EaseType::IN_OUT_SINE, c_main_);
	}

	// カメラが最初の視点へ戻り切るまでは、
	// TitleSelectを下の待機位置から動かさない
	titleSelectCameraElapsedTime_ += deltaTime;
	if (titleSelectCameraElapsedTime_ < kTitleSelectTransitionDuration_) {
		return;
	}

	// ========================================
	// Glass And Gin Exit / TitleSelect Entrance
	// ========================================

	titleSelectTransitionElapsedTime_ += deltaTime;

	float transitionT = titleSelectTransitionElapsedTime_ / kTitleSelectTransitionDuration_;
	if (transitionT > 1.0f) {
		transitionT = 1.0f;
	}

	const float smoothTransitionT = EaseInOut01(transitionT);

	// ジンは+Z側、氷を入れていたグラスは-Z側へ同時に退場する
	const float exitGinZ = -50.0f + kExitZDistance_ * smoothTransitionT;
	ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, exitGinZ));

	const float exitGlassZ = -60.0f - kExitZDistance_ * smoothTransitionT;
	glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, exitGlassZ));

	// TitleSelectはX・Zとスケールを保ち、下から現在のY位置へ上昇する
	const float titleSelectY = (8.0f - kTitleSelectRiseDistance_) * (1.0f - smoothTransitionT) + 8.0f * smoothTransitionT;

	titleSelectModels_[0].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectSelectedScale_, kTitleSelectSelectedScale_, kTitleSelectSelectedScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, titleSelectY, -60.0f));
	titleSelectModels_[1].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectNormalScale_, kTitleSelectNormalScale_, kTitleSelectNormalScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, titleSelectY, -50.0f));

	// 退場と登場が両方終わってから選択入力を有効にする
	if (transitionT >= 1.0f) {
		titlePhaseSelection_ = TitlePhaseSelection::Select;
		isTitleSelectInputEnabled_ = true;
	}
}

void TitlePhase::Start_CocktailCameraAnimation() {
	// ここで氷投入開始時の実際のカメラ位置を保存する。
	// Update_Animation側はこのZ値を変えず、X・Yと注視角だけを補間する。
	presentationCameraPosition_ = Game::Camera::Getter::GetWorldPosition(c_main_);
	iceTransferCameraStartFocus_ = Game::Camera::Getter::GetCenter(c_main_);
}

void TitlePhase::Start_GinCameraAnimation() {
	// 移動開始時の実際の位置と注視点を保存する。
	// Update_Animation側でY・Zを固定し、Xだけをプラス方向へ補間する。
	presentationCameraPosition_ = Game::Camera::Getter::GetWorldPosition(c_main_);
	ginCameraStartFocus_ = Game::Camera::Getter::GetCenter(c_main_);
}

void TitlePhase::Start_CocktailViewCameraAnimation() {
	const Vector3 cocktailFocus(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);

	// ジンの傾斜時間と同じ時間を使い、注視点・Phi・Thetaを同時に補間する。
	// Phiは水平0度・真上90度なので、pi/3は真上から30度傾いた角度になる。
	// Thetaを0へ戻すことで、カメラ位置もカクテルの正面側へ回り込む。
	Game::Camera::Setter::SetCenter(cocktailFocus, kGinTiltDuration_, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(kCocktailPeekCameraPhi_, kGinTiltDuration_, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(kCocktailFrontCameraTheta_, kGinTiltDuration_, EaseType::IN_OUT_SINE, c_main_);
}

void TitlePhase::Start_CocktailPeekCameraAnimation() {
	// 注視点と角度は既にカクテル用へ補間済みなので、
	// ここではその角度を維持し、Distanceだけを補間する。
	Game::Camera::Setter::SetDistance(kCocktailPeekCameraDistance_, kCocktailApproachCameraDuration_, EaseType::IN_OUT_SINE, c_main_);
}

void TitlePhase::Start_SideCameraAnimation() {
	// 最初と同じ、少し上から斜め下を見る視点へ戻す
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), kCameraMoveDuration_, EaseType::IN_OUT_SINE, c_main_);

	Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, kCameraMoveDuration_, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, kCameraMoveDuration_, EaseType::IN_OUT_SINE, c_main_);
}

void TitlePhase::Update_LightModels() {
	Update_Model(barModel_);
	Update_Model(glassModel_);
	Update_Model(CocktailModel_);
	Update_WaterModel();
	Update_TitleRayModel();
	Update_Model(ginModel_);
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Update_Model(iceModel_[i]);
	}
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		Update_Model(titleSelectModels_[i]);
	}
}

void TitlePhase::Update_TitleSelect() {
	// TitleSelectが下から上がり切るまでは、選択・確定入力を受け付けない
	if (titlePhaseSelection_ != TitlePhaseSelection::Select || !isTitleSelectInputEnabled_) {
		return;
	}

	const int32_t previousSelectedTitleIndex = selectedTitleIndex_;

	if (!isSelectionConfirmed_) {
		// Aキーまたは左矢印キーでZ=-60側を選択
		if (Game::IO::Key::IsJustPressed('A') || Game::IO::Key::IsJustPressed(0x25)) {
			selectedTitleIndex_ = 0;
		}

		// Dキーまたは右矢印キーでZ=-50側を選択
		if (Game::IO::Key::IsJustPressed('D') || Game::IO::Key::IsJustPressed(0x27)) {
			selectedTitleIndex_ = 1;
		}

		// Spaceキーで現在の選択を確定する
		if (Game::IO::Key::IsJustPressed(0x20)) {
			Start_SelectedCocktailAnimation();
		}
	}

	// Selectへ入った直後、または選択先が変わった時に、
	// 選択中のTitleSelectへ向けた上から下への光を最初から再生する。
	if (!isTitleSelectRayActive_ || selectedTitleIndex_ != previousSelectedTitleIndex) {
		isTitleSelectRayActive_ = true;
		titleSelectRayRevealElapsedTime_ = 0.0f;
		titleRayMaterialBuffer_.reveal = 0.0f;
	}

	const float selectedTitleZ = selectedTitleIndex_ == 0 ? -60.0f : -50.0f;
	titleRayPosition_ = Vector3(-70.0f, kTitleRayPositionY_, selectedTitleZ);
	titleRayModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), titleRayPosition_);

	const float pulsePhase = titleSelectPulseElapsedTime_ * (std::numbers::pi_v<float> * 2.0f) / kTitleSelectPulseDuration_;
	const float pulseScale = 1.0f + std::sin(pulsePhase) * kTitleSelectPulseAmplitude_;

	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		// 選択中と未選択の大きさの差を保ったまま、両方を常時拡大縮小する。
		const float baseScale = i == selectedTitleIndex_ ? kTitleSelectSelectedScale_ : kTitleSelectNormalScale_;
		const float scale = baseScale * pulseScale;
		const float z = i == 0 ? -60.0f : -50.0f;

		titleSelectModels_[i].transforms_[0] = EulerTransforms(Vector3(scale, scale, scale), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, 8.0f, z));
	}

	if (isSelectionConfirmed_) {
		Update_SelectedCocktailAnimation();
	}
}

void TitlePhase::Start_SelectedCocktailAnimation() {
	isSelectionConfirmed_ = true;
	selectionConfirmElapsedTime_ = 0.0f;
	selectionCameraElapsedTime_ = 0.0f;
	selectionCameraTheta_ = 0.0f;
	selectedCocktailTargetZ_ = selectedTitleIndex_ == 0 ? -60.0f : -50.0f;
	previousSelectionAnimationTime_ = std::chrono::steady_clock::now();

	// 必ず横視点から開始し、Update側で周回しながら真上へ補間する
	Game::Camera::Setter::SetPhiTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetDistance(kSelectionCameraStartDistance_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
}

void TitlePhase::Update_SelectedCocktailAnimation() {
	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration<float>(currentTime - previousSelectionAnimationTime_).count();
	previousSelectionAnimationTime_ = currentTime;

	if (deltaTime > 0.1f) {
		deltaTime = 0.1f;
	}

	selectionConfirmElapsedTime_ += deltaTime;
	selectionCameraElapsedTime_ += deltaTime;
	float moveT = selectionConfirmElapsedTime_ / kSelectionConfirmMoveDuration_;
	if (moveT > 1.0f) {
		moveT = 1.0f;
	}
	const float smoothMoveT = EaseInOut01(moveT);
	const float currentCocktailZ = kIceTargetCenterZ_ * (1.0f - smoothMoveT) + selectedCocktailTargetZ_ * smoothMoveT;
	const float zOffset = currentCocktailZ - kIceTargetCenterZ_;

	// カクテルグラス・液体・全ての氷を、配置を崩さず選択中のZへ移動する
	CocktailModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(kIceTargetCenterX_, 7.0f, currentCocktailZ));
	cocktailWaterModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(kIceTargetCenterX_, 7.0f, currentCocktailZ));

	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		const Vector3 icePosition = iceTargetPositions_[i] + Vector3(0.0f, 0.0f, zOffset);
		iceModel_[i].transforms_[0] = EulerTransforms(Vector3(0.2f, 0.2f, 0.2f), Vector3(0.0f, -std::numbers::pi_v<float> / 2.0f, 0.0f), icePosition);
	}

	// カメラの注視点は、選択位置へ移動中のカクテルを追従する
	Game::Camera::Setter::SetCenter(Vector3(kIceTargetCenterX_, 7.0f, currentCocktailZ), 0.0f, EaseType::IN_OUT_SINE, c_main_);

	if (selectionCameraElapsedTime_ < kSelectionCameraRiseDuration_) {
		// 横向きから真上へ滑らかに上がりながら、カクテルの周囲を2周する
		const float riseT = selectionCameraElapsedTime_ / kSelectionCameraRiseDuration_;
		const float smoothRiseT = EaseInOut01(riseT);
		selectionCameraTheta_ = kSelectionCameraOrbitAngle_ * smoothRiseT;

		Game::Camera::Setter::SetPhiTarget(kSelectionCameraTopPhi_ * smoothRiseT, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetThetaTarget(selectionCameraTheta_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetDistance(kSelectionCameraStartDistance_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		return;
	}

	// 真上へ到達した後は回転角を固定し、カクテルへ近づく
	float approachT = (selectionCameraElapsedTime_ - kSelectionCameraRiseDuration_) / kSelectionCameraApproachDuration_;
	if (approachT > 1.0f) {
		approachT = 1.0f;
	}
	const float smoothApproachT = EaseInOut01(approachT);
	const float cameraDistance = kSelectionCameraStartDistance_ * (1.0f - smoothApproachT) + kSelectionCameraStopDistance_ * smoothApproachT;

	Game::Camera::Setter::SetPhiTarget(kSelectionCameraTopPhi_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(kSelectionCameraOrbitAngle_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetDistance(cameraDistance, 0.0f, EaseType::IN_OUT_SINE, c_main_);
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

void TitlePhase::Update_WaterModel() {
	cocktailWaterModel_.worldMatrices_[0] = cocktailWaterModel_.transforms_[0].GetWorldMatrix();

	WaterTransformBuffer transformBuffer{};
	transformBuffer.world = cocktailWaterModel_.worldMatrices_[0];

	// この演出ではXYZを同じ値で拡大するため、正規化後の法線方向はworld行列でも一致する
	transformBuffer.worldInverseTranspose = cocktailWaterModel_.worldMatrices_[0];
	transformBuffer.viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);

	waterWaveBuffer_.motionWaveTime = cocktailWaterAnimationTime_;
	waterColorBuffer_.motionWaveTime = cocktailWaterAnimationTime_;

	// 現在のカメラの実ワールド座標をスペキュラとフレネルへ渡す
	waterCameraBuffer_.cameraPositionWS = Game::Camera::Getter::GetWorldPosition(c_main_);

	// WaterSurface.VS.hlsl
	cocktailWaterModel_.Models_->SetCBufferData(0, ShaderType::VertexShader, &transformBuffer);
	cocktailWaterModel_.Models_->SetCBufferData(1, ShaderType::VertexShader, &waterWaveBuffer_);

	// WaterSurface.PS.hlsl
	cocktailWaterModel_.Models_->SetCBufferData(0, ShaderType::PixelShader, &waterCameraBuffer_);
	cocktailWaterModel_.Models_->SetCBufferData(1, ShaderType::PixelShader, &waterColorBuffer_);
	cocktailWaterModel_.Models_->SetCBufferData(2, ShaderType::PixelShader, &waterLightingBuffer_);
}

void TitlePhase::Update_TitleRayModel() {
	titleRayModel_.worldMatrices_[0] = titleRayModel_.transforms_[0].GetWorldMatrix();

	titleRayCameraBuffer_.viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	titleRayCameraBuffer_.cameraPositionWS = Game::Camera::Getter::GetWorldPosition(c_main_);
	titleRayCameraBuffer_.padding = 0.0f;

	titleRayObjectBuffer_.world = titleRayModel_.worldMatrices_[0];

	// TitleRayは回転なし・等倍なので、現在位置の符号を反転して逆行列を作る。
	// Select中に光が移動してもworldToObjectが同じ位置へ追従する。
	const EulerTransforms inverseTitleRayTransform(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-titleRayPosition_.x, -titleRayPosition_.y, -titleRayPosition_.z));
	titleRayObjectBuffer_.worldToObject = inverseTitleRayTransform.GetWorldMatrix();

	// SpotLight.PS.hlsl : CameraCB b0 / ObjectCB b1 / MaterialCB b2
	titleRayModel_.Models_->SetCBufferData(0, ShaderType::VertexShader, &titleRayCameraBuffer_);
	titleRayModel_.Models_->SetCBufferData(1, ShaderType::VertexShader, &titleRayObjectBuffer_);
	titleRayModel_.Models_->SetCBufferData(2, ShaderType::VertexShader, &titleRayMaterialBuffer_);

	titleRayModel_.Models_->SetCBufferData(0, ShaderType::PixelShader, &titleRayCameraBuffer_);
	titleRayModel_.Models_->SetCBufferData(1, ShaderType::PixelShader, &titleRayObjectBuffer_);
	titleRayModel_.Models_->SetCBufferData(2, ShaderType::PixelShader, &titleRayMaterialBuffer_);
}

void TitlePhase::Draw_LightModels() {
	barModel_.Models_->Draw();
	glassModel_.Models_->Draw();
	CocktailModel_.Models_->Draw();
	cocktailWaterModel_.Models_->Draw();
	ginModel_.Models_->Draw();
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		iceModel_[i].Models_->Draw();
	}

	// 切り替え開始から表示し、下から上昇させる
	if (isTitleSelectTransitionStarted_ || titlePhaseSelection_ == TitlePhaseSelection::Select) {
		for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
			titleSelectModels_[i].Models_->Draw();
		}
	}

	// TitleRayが深度を書き込んでもTitleSelectを隠さないように、
	// TitleSelectを含む全モデルの描画後にボリューム光を重ねる。
	titleRayModel_.Models_->Draw();
}
