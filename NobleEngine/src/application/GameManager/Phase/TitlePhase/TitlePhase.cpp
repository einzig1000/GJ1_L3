#include "TitlePhase.h"

#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <cmath>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
#include <string>
TitlePhase::TitlePhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("SimpleModels");
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetDistance(20.0f, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetFovTarget(0.65f, 0.0f, EaseType::IN_BACK, c_main_);
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
	titleRayModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(kTitleRayPositionX_, kTitleRayPositionY_, kTitleRayPositionZ_));
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
	titleRayRevealElapsedTime_ = 0.0f;
	isTitleRayRevealFinished_ = false;
	titleRayMaterialBuffer_.reveal = 0.0f;
	preLiftElapsedTime_ = 0.0f;
	isPreLiftFinished_ = false;
	glassTiltElapsedTime_ = 0.0f;
	isGlassTiltFinished_ = false;
	glassTiltHoldElapsedTime_ = 0.0f;
	isGlassTiltHoldFinished_ = false;
	iceAnimationElapsedTime_ = 0.0f;
	isIceAnimationFinished_ = false;
	isCocktailCameraStarted_ = false;
	isSideCameraReturned_ = false;
	glassReturnElapsedTime_ = 0.0f;
	isGlassReturnFinished_ = false;
	ginAnimationElapsedTime_ = 0.0f;
	isGinCameraStarted_ = false;
	postGinWaitElapsedTime_ = 0.0f;
	postGinCameraElapsedTime_ = 0.0f;
	isPostGinOverheadCameraStarted_ = false;
	titleSelectCameraElapsedTime_ = 0.0f;
	titleSelectTransitionElapsedTime_ = 0.0f;
	isTitleSelectTransitionStarted_ = false;
	isTitleSelectInputEnabled_ = false;
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

	previousAnimationTime_ = std::chrono::steady_clock::now();
	previousSelectionAnimationTime_ = previousAnimationTime_;
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
	Game::Camera::Setter::SetCenter(target, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetPhiTarget(phi, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(theta, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetDistance(distance, 0.0f, EaseType::IN_BACK, c_main_);
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

	// ジンのアニメーションまで完了した後は、水面時間だけ更新して終了
	if (titlePhaseSelection_ == TitlePhaseSelection::Select) {
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
		titleRayMaterialBuffer_.reveal = revealT * revealT * (3.0f - 2.0f * revealT);
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
		const float smoothEntranceT = entranceT * entranceT * (3.0f - 2.0f * entranceT);

		const float glassZ = (-60.0f - kEntranceZDistance_) * (1.0f - smoothEntranceT) + (-60.0f) * smoothEntranceT;
		glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, glassZ));

		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			const Vector3 entranceStart = iceStartPositions_[i] + Vector3(0.0f, 0.0f, -kEntranceZDistance_);
			const Vector3 icePosition = entranceStart * (1.0f - smoothEntranceT) + iceStartPositions_[i] * smoothEntranceT;

			iceModel_[i].transforms_[0] = EulerTransforms(Vector3(0.2f, 0.2f, 0.2f), Vector3(0.0f, 0.0f, 0.0f), icePosition);
		}

		const float ginZ = (-50.0f + kEntranceZDistance_) * (1.0f - smoothEntranceT) + (-50.0f) * smoothEntranceT;
		ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, ginZ));

		// 全員が定位置へ到着してから持ち上げ演出へ進む
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

		float liftT = preLiftElapsedTime_ / kPreLiftDuration_;
		if (liftT >= 1.0f) {
			liftT = 1.0f;
			isPreLiftFinished_ = true;
		}

		const float liftOffsetY = kGlassLiftHeight_ * liftT;
		const Vector3 iceScale(0.2f, 0.2f, 0.2f);
		const Vector3 iceRotate(0.0f, 0.0f, 0.0f);

		// 氷の縦一列の形を保ったまま、グラスと一緒に上へ持ち上げる
		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			const Vector3 liftedPosition = iceStartPositions_[i] + Vector3(0.0f, liftOffsetY, 0.0f);
			iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, liftedPosition);
		}

		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(0.0f, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f + liftOffsetY, -60.0f);
		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);

		// 現在の向きから氷用グラスへ、カメラ位置を固定したまま滑らかに向きを変える
		float glassCameraT = preLiftElapsedTime_ / kGlassCameraLookDuration_;
		if (glassCameraT > 1.0f) {
			glassCameraT = 1.0f;
		}
		const float smoothGlassCameraT = glassCameraT * glassCameraT * (3.0f - 2.0f * glassCameraT);
		const Vector3 glassCameraFocus = glassCameraStartFocus_ * (1.0f - smoothGlassCameraT) + glassPosition * smoothGlassCameraT;
		AimCameraFromFixedPosition(presentationCameraPosition_, glassCameraFocus);

		// 持ち上げが終わってから氷の移動を開始する
		return;
	}

	// ========================================
	// Glass Tilt And Ice Move Animation
	// ========================================

	if (!isIceAnimationFinished_) {
		iceAnimationElapsedTime_ += deltaTime;
		glassTiltElapsedTime_ += deltaTime;

		float iceT = iceAnimationElapsedTime_ / kIceMoveDuration_;
		if (iceT >= 1.0f) {
			iceT = 1.0f;
			isIceAnimationFinished_ = true;
		}

		float tiltT = glassTiltElapsedTime_ / kGlassTiltDuration_;
		if (tiltT >= 1.0f) {
			tiltT = 1.0f;
			isGlassTiltFinished_ = true;
		}
		const float smoothTiltT = tiltT * tiltT * (3.0f - 2.0f * tiltT);
		const float smoothIceT = iceT * iceT * (3.0f - 2.0f * iceT);
		const float currentTiltAngle = kGlassTiltAngle_ * smoothTiltT;
		const float tiltCos = std::cos(currentTiltAngle);
		const float tiltSin = std::sin(currentTiltAngle);
		const float glassPivotY = 7.0f + kGlassLiftHeight_;
		const float glassPivotZ = kIceStartZ_;

		const Vector3 iceScale(0.2f, 0.2f, 0.2f);

		const float targetIceRotationY = -std::numbers::pi_v<float> * 0.5f;
		const Vector3 iceRotate(currentTiltAngle * (1.0f - smoothIceT), targetIceRotationY * smoothIceT, 0.0f);

		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			const float liftedIceY = kIceStartBottomY_ + static_cast<float>(i) * kIceVerticalSpacing_ + kGlassLiftHeight_;
			const float relativeY = liftedIceY - glassPivotY;
			const float tiltedY = glassPivotY + relativeY * tiltCos;
			const float tiltedZ = glassPivotZ + relativeY * tiltSin;
			const Vector3 currentTiltedPosition(kIceStartX_, tiltedY, tiltedZ);

			iceTiltedStartPositions_[i] = currentTiltedPosition;

			// グラスの傾斜と同時に、現在の傾斜位置からカクテルへ移動する
			const Vector3 linearPosition = currentTiltedPosition * (1.0f - smoothIceT) + iceTargetPositions_[i] * smoothIceT;

			// t=0とt=1では高さ0、中央のt=0.5で最大になる放物線
			const float arcOffsetY = 4.0f * kIceArcHeight_ * iceT * (1.0f - iceT);
			const Vector3 position = linearPosition + Vector3(0.0f, arcOffsetY, 0.0f);

			iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, position);
		}

		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(currentTiltAngle, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f + kGlassLiftHeight_, -60.0f);

		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);

		// 注視点を氷用グラスからカクテルグラスへ、氷の移動率に合わせて動かす
		const Vector3 sourceGlassFocus(-60.0f, 7.0f + kGlassLiftHeight_, -60.0f);
		const Vector3 cocktailFocus(kIceTargetCenterX_, 7.0f, kIceTargetCenterZ_);
		const Vector3 cameraFocus = sourceGlassFocus * (1.0f - smoothIceT) + cocktailFocus * smoothIceT;
		AimCameraFromFixedPosition(presentationCameraPosition_, cameraFocus);

		if (!isIceAnimationFinished_) {
			return;
		}

		isCocktailCameraStarted_ = true;
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
		const float smoothReturnT = returnT * returnT * (3.0f - 2.0f * returnT);
		const float remainingAmount = 1.0f - smoothReturnT;

		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(kGlassTiltAngle_ * remainingAmount, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f + kGlassLiftHeight_ * remainingAmount, -60.0f);

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
	const Vector3 ginPosition(-60.0f, 7.0f, -50.0f);
	const Vector3 cocktailFocus(kIceTargetCenterX_, 1.5f, kIceTargetCenterZ_);

	// ジンが傾き始めると同時に、カメラをジンへ向ける
	if (!isGinCameraStarted_) {
		isGinCameraStarted_ = true;
		ginCameraStartFocus_ = Game::Camera::Getter::GetCenter(c_main_);
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
		ginTiltAmount = tiltT * tiltT * (3.0f - 2.0f * tiltT);

		// ジンの傾斜とは別の時間で、直前の向きからジンへ滑らかに向きを変える
		float ginCameraT = ginAnimationElapsedTime_ / kGinCameraMoveDuration_;
		if (ginCameraT > 1.0f) {
			ginCameraT = 1.0f;
		}
		const float smoothGinCameraT = ginCameraT * ginCameraT * (3.0f - 2.0f * ginCameraT);
		const Vector3 cameraFocus = ginCameraStartFocus_ * (1.0f - smoothGinCameraT) + ginPosition * smoothGinCameraT;
		AimCameraFromFixedPosition(presentationCameraPosition_, cameraFocus);
	} else if (ginAnimationElapsedTime_ < ginHoldEndTime) {
		// 最大まで傾いた姿勢を1秒間維持する
		ginTiltAmount = 1.0f;

		float holdT = (ginAnimationElapsedTime_ - kGinTiltDuration_) / kGinTiltHoldDuration_;
		if (holdT > 1.0f) {
			holdT = 1.0f;
		}
		const float smoothHoldT = holdT * holdT * (3.0f - 2.0f * holdT);

		// ジンが停止している1秒の間に、注視点をカクテルグラスへ移す
		const Vector3 cameraFocus = ginPosition * (1.0f - smoothHoldT) + cocktailFocus * smoothHoldT;
		AimCameraFromFixedPosition(presentationCameraPosition_, cameraFocus);
	} else if (ginAnimationElapsedTime_ < ginAnimationEndTime) {
		// 1秒停止した後、元の姿勢へ滑らかに戻す
		float returnT = (ginAnimationElapsedTime_ - ginHoldEndTime) / kGinReturnDuration_;
		if (returnT > 1.0f) {
			returnT = 1.0f;
		}
		const float smoothReturnT = returnT * returnT * (3.0f - 2.0f * returnT);
		ginTiltAmount = 1.0f - smoothReturnT;

		// 復帰中もカメラはカクテルグラスへ向けたままにする
		AimCameraFromFixedPosition(presentationCameraPosition_, cocktailFocus);
	}

	const Vector3 ginRotate(-std::numbers::pi_v<float> / 3.0f * ginTiltAmount, 0.0f, 0.0f);

	ginModel_.transforms_[0] = EulerTransforms(ginScale, ginRotate, ginPosition);

	// ジンが最大まで傾いた時点から液体を表示する
	if (ginAnimationElapsedTime_ >= kGinTiltDuration_) {
		isCocktailWaterAppearing_ = true;
		cocktailWaterScaleElapsedTime_ += deltaTime;

		float waterScaleT = cocktailWaterScaleElapsedTime_ / kCocktailWaterScaleDuration_;
		if (waterScaleT > 1.0f) {
			waterScaleT = 1.0f;
		}

		const float smoothWaterScaleT = waterScaleT * waterScaleT * (3.0f - 2.0f * waterScaleT);
		const float waterScale = 10.0f * smoothWaterScaleT;

		cocktailWaterModel_.transforms_[0] = EulerTransforms(Vector3(waterScale, waterScale, waterScale), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	}

	// 傾斜、1秒停止、復帰がすべて終わるまで次の演出へ進まない
	if (ginAnimationElapsedTime_ < ginAnimationEndTime) {
		return;
	}

	// ジンの傾斜演出がすべて終わったら、カクテルグラスの真上へ移動する
	if (!isPostGinOverheadCameraStarted_) {
		isPostGinOverheadCameraStarted_ = true;
		postGinCameraElapsedTime_ = 0.0f;

		Game::Camera::Setter::SetCenter(cocktailFocus, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetPhiTarget(std::numbers::pi_v<float> / 2.0f, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetThetaTarget(0.0f, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetDistance(kPostGinOverheadDistance_, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
	}

	postGinCameraElapsedTime_ += deltaTime;
	if (postGinCameraElapsedTime_ < kCameraMoveDuration_) {
		return;
	}

	// 真上へ移動した後は、Spaceキーが押されるまで次の状態へ進まない
	if (!isTitleSelectTransitionStarted_) {
		if (!Game::IO::Key::IsJustPressed(0x20)) {
			return;
		}

		isTitleSelectTransitionStarted_ = true;
		titleSelectCameraElapsedTime_ = 0.0f;
		titleSelectTransitionElapsedTime_ = 0.0f;

		// 先にカメラだけを最初の状態へ戻す
		Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), kTitleSelectTransitionDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, kTitleSelectTransitionDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetThetaTarget(0.0f, kTitleSelectTransitionDuration_, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetDistance(20.0f, kTitleSelectTransitionDuration_, EaseType::IN_BACK, c_main_);
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

	const float smoothTransitionT = transitionT * transitionT * (3.0f - 2.0f * transitionT);

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
	// 氷がカクテルへ入った後に使用する、現在の上側視点
	Game::Camera::Setter::SetCenter(Vector3(-62.0f, 6.0f, -55.0f), kCameraMoveDuration_, EaseType::IN_BACK, c_main_);

	// 上下方向はPhiで回転させる。Thetaは横方向なので0のまま固定する
	Game::Camera::Setter::SetPhiTarget(std::numbers::pi_v<float> / 2.0f, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
}

void TitlePhase::Start_SideCameraAnimation() {
	// 最初と同じ、少し上から斜め下を見る視点へ戻す
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -55.0f), kCameraMoveDuration_, EaseType::IN_BACK, c_main_);

	Game::Camera::Setter::SetPhiTarget(kInitialCameraPhi_, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, kCameraMoveDuration_, EaseType::IN_BACK, c_main_);
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

	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		const float scale = i == selectedTitleIndex_ ? kTitleSelectSelectedScale_ : kTitleSelectNormalScale_;
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
	Game::Camera::Setter::SetPhiTarget(0.0f, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetDistance(kSelectionCameraStartDistance_, 0.0f, EaseType::IN_BACK, c_main_);
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
	const float smoothMoveT = moveT * moveT * (3.0f - 2.0f * moveT);
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
	Game::Camera::Setter::SetCenter(Vector3(kIceTargetCenterX_, 7.0f, currentCocktailZ), 0.0f, EaseType::IN_BACK, c_main_);

	if (selectionCameraElapsedTime_ < kSelectionCameraRiseDuration_) {
		// 横向きから真上へ滑らかに上がりながら、カクテルの周囲を2周する
		const float riseT = selectionCameraElapsedTime_ / kSelectionCameraRiseDuration_;
		const float smoothRiseT = riseT * riseT * (3.0f - 2.0f * riseT);
		selectionCameraTheta_ = kSelectionCameraOrbitAngle_ * smoothRiseT;

		Game::Camera::Setter::SetPhiTarget(kSelectionCameraTopPhi_ * smoothRiseT, 0.0f, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetThetaTarget(selectionCameraTheta_, 0.0f, EaseType::IN_BACK, c_main_);
		Game::Camera::Setter::SetDistance(kSelectionCameraStartDistance_, 0.0f, EaseType::IN_BACK, c_main_);
		return;
	}

	// 真上へ到達した後は回転角を固定し、カクテルへ近づく
	float approachT = (selectionCameraElapsedTime_ - kSelectionCameraRiseDuration_) / kSelectionCameraApproachDuration_;
	if (approachT > 1.0f) {
		approachT = 1.0f;
	}
	const float smoothApproachT = approachT * approachT * (3.0f - 2.0f * approachT);
	const float cameraDistance = kSelectionCameraStartDistance_ * (1.0f - smoothApproachT) + kSelectionCameraStopDistance_ * smoothApproachT;

	Game::Camera::Setter::SetPhiTarget(kSelectionCameraTopPhi_, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetThetaTarget(kSelectionCameraOrbitAngle_, 0.0f, EaseType::IN_BACK, c_main_);
	Game::Camera::Setter::SetDistance(cameraDistance, 0.0f, EaseType::IN_BACK, c_main_);
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

	// TitleRayは回転なし・等倍で固定しているため、逆行列は平行移動の符号反転で求められる
	const EulerTransforms inverseTitleRayTransform(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-kTitleRayPositionX_, -kTitleRayPositionY_, -kTitleRayPositionZ_));
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

	// 不透明モデルを描き終えた後に、カクテル上のボリューム光を重ねる
	titleRayModel_.Models_->Draw();

	// 切り替え開始から表示し、下から上昇させる
	if (isTitleSelectTransitionStarted_ || titlePhaseSelection_ == TitlePhaseSelection::Select) {
		for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
			titleSelectModels_[i].Models_->Draw();
		}
	}
}
