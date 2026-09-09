#include "ResultPhase.h"
#include "Game.h"

#include <algorithm>
#include <cmath>
#include <numbers>

ResultPhase::ResultPhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("Result");
	Game::Camera::Setter::SetCenter(Vector3(-50.0f, 7.0f, -10.0f), 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);

	// モデル
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
	signboardModel_.ID = Game::Asset::Model::Load("assets/application/model/Signboard/Signboard.obj");
	// TitlePhaseで使用しているカクテルグラスと同じモデル
	cocktailModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Cocktail/Cocktail.obj");
	cocktailWaterModel_.ID = Game::Asset::Model::Load("assets/application/model/Water/CocktailWater.obj");
	resultRayModel_.ID = Game::Asset::Model::Load("assets/application/model/Title_Select/TitleRay.obj");

	// テクスチャ
	barModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");
	signboardModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Signboard/Signboard.png");
	cocktailModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Alcohol/Cocktail/Cocktail.png");

	// Manモデル・テクスチャ・アニメーション
	manModelID_ = Game::Asset::Model::Load("assets/application/model/Man/man.gltf");
	manTextureID_ = Game::Asset::Texture::Load("assets/application/model/Man/texture_body.png");
	manIdleAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Idle");
	manWalkAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Walk");
	manGoodAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Good");
	manBadAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "BAD");
	manCatchAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Catch ");
	manThrowAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Throw");
}
ResultPhase::~ResultPhase() {}
void ResultPhase::Initialize() {
	// 結果画面の初期化処理をここに記述
	InitializeCommon();
	if (isWin_) {
		InitializeWin();
	} else {
		InitializeLose();
	}
}
void ResultPhase::Update() {
	// 結果画面の更新処理をここに記述
	UpdateCommon();
	if (isWin_) {
		UpdateWin();
	} else {
		UpdateLose();
	}
}
void ResultPhase::Draw() {
	// 結果画面の描画処理をここに記述
	DrawCommon();
	if (isWin_) {
		DrawWin();
	} else {
		DrawLose();
	}
}
void ResultPhase::DrawImGui() {
	// ImGuiを使用した結果画面の描画処理をここに記述
	DrawImGui_Models();
}

void ResultPhase::Initialize_LightModel(Model& model) {
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

void ResultPhase::Initialize_ResultRayModel() {
	resultRayModel_.Models_ = std::make_unique<RenderObject>();
	resultRayModel_.Models_->psoConfig_.vs = "assets/shaders/SpotLightRay/SpotLightRay.VS.hlsl";
	resultRayModel_.Models_->psoConfig_.ps = "assets/shaders/SpotLightRay/SpotLightRay.PS.hlsl";
	resultRayModel_.Models_->SetupFromShaders();

	resultRayModel_.Models_->modelID_ = resultRayModel_.ID;
	resultRayModel_.Models_->instanceNum_ = 1;
	resultRayModel_.transforms_.resize(1, EulerTransforms());
	resultRayModel_.worldMatrices_.resize(1, Matrix4x4());

	resultRayModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), resultRayPosition_);

	// TitlePhaseのTitleRayと同じ見た目を使う
	// 幅を広げ、背景やモデルが見える薄めのピンク色にする
	resultRayMaterialBuffer_.color = Vector4(0.74f, 0.28f, 0.39f, 0.10f);
	resultRayMaterialBuffer_.intensity = 1.8f;
	resultRayMaterialBuffer_.tipRadius = 7.0f;
	resultRayMaterialBuffer_.endRadius = 12.5f;
	resultRayMaterialBuffer_.coneLength = 16.0f;
	// GoodでManの手が上がるまでは消灯しておく
	resultRayMaterialBuffer_.reveal = 0.0f;
	resultRayMaterialBuffer_.revealSoftness = 0.08f;
	resultRayMaterialBuffer_.density = 0.65f;
	resultRayMaterialBuffer_.centerBrightness = 1.25f;
	resultRayMaterialBuffer_.edgeSoftness = 0.35f;
	resultRayMaterialBuffer_.distanceFade = 0.35f;
	resultRayMaterialBuffer_.stepCount = 48;
	resultRayMaterialBuffer_.padding = 0.0f;
}

void ResultPhase::Initialize_CocktailWaterModel() {
	cocktailWaterModel_.Models_ = std::make_unique<RenderObject>();
	cocktailWaterModel_.Models_->psoConfig_.vs = "assets/shaders/WaterSurface/WaterSurface.VS.hlsl";
	cocktailWaterModel_.Models_->psoConfig_.ps = "assets/shaders/WaterSurface/WaterSurface.PS.hlsl";
	cocktailWaterModel_.Models_->SetupFromShaders();
	cocktailWaterModel_.Models_->modelID_ = cocktailWaterModel_.ID;
	cocktailWaterModel_.Models_->instanceNum_ = 1;
	cocktailWaterModel_.transforms_.resize(1, EulerTransforms());
	cocktailWaterModel_.worldMatrices_.resize(1, Matrix4x4());

	// TitlePhaseの完成後のCocktailWaterと同じ設定
	waterWaveBuffer_.relativeScale = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
	waterWaveBuffer_.waveAxisXWS = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	waterWaveBuffer_.waveAxisYWS = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	waterWaveBuffer_.waveAxisZWS = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
	waterWaveBuffer_.surfaceY = cocktailPosition_.y;
	waterWaveBuffer_.commonWorldScale = cocktailScale_.x;
	waterWaveBuffer_.sideWaveDepth = 0.5f;
	waterWaveBuffer_.waveHeight = 0.02f;
	waterWaveBuffer_.waveFrequency = 2.0f;
	waterWaveBuffer_.waveSpeed = 1.0f;
	waterWaveBuffer_.motionHeightBoost = 0.35f;
	waterWaveBuffer_.motionIntensity = 0.0f;
	waterWaveBuffer_.motionWaveTime = 0.0f;

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
	waterColorBuffer_.motionWaveTime = 0.0f;
	waterColorBuffer_.smoothness = 0.8f;
	waterColorBuffer_.fresnelStrength = 0.2f;

	waterLightingBuffer_.mainLightDirection = Vector4(0.0f, 0.70710678f, -0.70710678f, 0.0f);
	waterLightingBuffer_.mainLightColor = Vector4(1.0f, 0.35f, 0.05f, 1.0f);
	waterLightingBuffer_.ambientSky = Vector4(0.15f, 0.15f, 0.18f, 1.0f);
	waterLightingBuffer_.ambientGround = Vector4(0.05f, 0.04f, 0.04f, 1.0f);
	cocktailWaterAnimationTime_ = 0.0f;
}

void ResultPhase::Update_CocktailWaterModel(float deltaTime) {
	if (cocktailWaterModel_.Models_ == nullptr || cocktailWaterModel_.transforms_.empty()) {
		return;
	}

	cocktailWaterAnimationTime_ += deltaTime;
	cocktailWaterModel_.transforms_[0].scale = cocktailScale_;
	cocktailWaterModel_.transforms_[0].translate = cocktailPosition_;
	cocktailWaterModel_.worldMatrices_[0] = cocktailWaterModel_.transforms_[0].GetWorldMatrix();

	WaterTransformBuffer transformBuffer{};
	transformBuffer.world = cocktailWaterModel_.worldMatrices_[0];
	// カクテルと同じXYZ等倍スケールなので、法線方向には同じ行列を使用できる
	transformBuffer.worldInverseTranspose = cocktailWaterModel_.worldMatrices_[0];
	transformBuffer.viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);

	waterWaveBuffer_.surfaceY = cocktailPosition_.y;
	waterWaveBuffer_.commonWorldScale = cocktailScale_.x;
	waterWaveBuffer_.motionWaveTime = cocktailWaterAnimationTime_;
	waterColorBuffer_.motionWaveTime = cocktailWaterAnimationTime_;
	waterCameraBuffer_.cameraPositionWS = Game::Camera::Getter::GetWorldPosition(c_main_);
	waterCameraBuffer_.padding = 0.0f;

	cocktailWaterModel_.Models_->SetCBufferData(0, ShaderType::VertexShader, &transformBuffer);
	cocktailWaterModel_.Models_->SetCBufferData(1, ShaderType::VertexShader, &waterWaveBuffer_);
	cocktailWaterModel_.Models_->SetCBufferData(0, ShaderType::PixelShader, &waterCameraBuffer_);
	cocktailWaterModel_.Models_->SetCBufferData(1, ShaderType::PixelShader, &waterColorBuffer_);
	cocktailWaterModel_.Models_->SetCBufferData(2, ShaderType::PixelShader, &waterLightingBuffer_);
}

void ResultPhase::Update_ResultRayAnimation(float deltaTime) {
	// WinはGoodで手を上げた時刻、LoseはBadへ入った時点でResultRayを点灯する。
	const bool shouldTurnOnWinRay = isWin_ && ((manWinState_ == ManWinState::PlayingGood && manAnimationTime_ >= kResultRayTurnOnGoodTime_) || manWinState_ == ManWinState::HoldingGood);
	const bool shouldTurnOnLoseRay = !isWin_ && manLoseState_ != ManLoseState::Walking;

	if (!isResultRayTurnedOn_ && (shouldTurnOnWinRay || shouldTurnOnLoseRay)) {
		isResultRayTurnedOn_ = true;
		resultRayRevealElapsedTime_ = 0.0f;
	}

	if (!isResultRayTurnedOn_) {
		resultRayMaterialBuffer_.reveal = 0.0f;
		return;
	}

	resultRayRevealElapsedTime_ += deltaTime;
	resultRayMaterialBuffer_.reveal = EaseInOut01(resultRayRevealElapsedTime_ / kResultRayRevealDuration_);
}

void ResultPhase::Update_ResultRayModel() {
	if (resultRayModel_.Models_ == nullptr || resultRayModel_.transforms_.empty()) {
		return;
	}

	// ヘッダーまたはImGuiで変更した位置を毎フレーム反映する
	resultRayModel_.transforms_[0].translate = resultRayPosition_;
	resultRayModel_.worldMatrices_[0] = resultRayModel_.transforms_[0].GetWorldMatrix();

	resultRayCameraBuffer_.viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	resultRayCameraBuffer_.cameraPositionWS = Game::Camera::Getter::GetWorldPosition(c_main_);
	resultRayCameraBuffer_.padding = 0.0f;

	resultRayObjectBuffer_.world = resultRayModel_.worldMatrices_[0];

	// ResultRayは回転なし・等倍なので、平行移動を反転して逆行列を作る
	const EulerTransforms inverseResultRayTransform(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-resultRayPosition_.x, -resultRayPosition_.y, -resultRayPosition_.z));
	resultRayObjectBuffer_.worldToObject = inverseResultRayTransform.GetWorldMatrix();

	resultRayModel_.Models_->SetCBufferData(0, ShaderType::VertexShader, &resultRayCameraBuffer_);
	resultRayModel_.Models_->SetCBufferData(1, ShaderType::VertexShader, &resultRayObjectBuffer_);
	resultRayModel_.Models_->SetCBufferData(2, ShaderType::VertexShader, &resultRayMaterialBuffer_);

	resultRayModel_.Models_->SetCBufferData(0, ShaderType::PixelShader, &resultRayCameraBuffer_);
	resultRayModel_.Models_->SetCBufferData(1, ShaderType::PixelShader, &resultRayObjectBuffer_);
	resultRayModel_.Models_->SetCBufferData(2, ShaderType::PixelShader, &resultRayMaterialBuffer_);
}

void ResultPhase::Draw_ResultRayModel() {
	if (resultRayModel_.Models_ == nullptr || resultRayMaterialBuffer_.reveal <= 0.0001f) {
		return;
	}

	// 半透明の光線なので、通常モデルを描いた後に重ねる
	resultRayModel_.Models_->Draw();
}

float ResultPhase::EaseInOut01(float value) {
	const float t = std::clamp(value, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

void ResultPhase::AimCameraFromPosition(const Vector3& cameraPosition, const Vector3& target) {
	const float directionX = target.x - cameraPosition.x;
	const float directionY = target.y - cameraPosition.y;
	const float directionZ = target.z - cameraPosition.z;
	const float distance = std::sqrt(directionX * directionX + directionY * directionY + directionZ * directionZ);
	if (distance <= 0.0001f) {
		return;
	}

	const float normalizedY = std::clamp(directionY / distance, -1.0f, 1.0f);
	const float phi = std::asin(-normalizedY);
	const float theta = std::atan2(-directionZ, -directionX);

	Game::Camera::Setter::SetCenter(target, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(phi, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(theta, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetDistanceTarget(distance, 0.0f, EaseType::IN_OUT_SINE, c_main_);
}

void ResultPhase::Start_ResultCameraAnimation() {
	const Vector3 cameraPosition = Game::Camera::Getter::GetWorldPosition(c_main_);
	const float directionX = cocktailPosition_.x - cameraPosition.x;
	const float directionY = cocktailPosition_.y - cameraPosition.y;
	const float directionZ = cocktailPosition_.z - cameraPosition.z;

	resultCameraStartDistance_ = std::sqrt(directionX * directionX + directionY * directionY + directionZ * directionZ);
	if (resultCameraStartDistance_ <= 0.0001f) {
		resultCameraStartDistance_ = 1.0f;
	}

	resultCameraStartPhi_ = std::asin(-std::clamp(directionY / resultCameraStartDistance_, -1.0f, 1.0f));
	resultCameraStartTheta_ = std::atan2(-directionZ, -directionX);
	resultCameraElapsedTime_ = 0.0f;
	resultCameraState_ = ResultCameraState::Orbiting;
}

void ResultPhase::Update_ResultCameraAnimation(float deltaTime) {
	if (resultCameraState_ == ResultCameraState::WaitingForInput) {
		// Goodの最終ポーズになるまではSpaceを受け付けない。
		if (manWinState_ == ManWinState::HoldingGood && Game::IO::Key::IsJustPressed(0x20)) {
			Start_ResultCameraAnimation();
		}
		return;
	}

	if (resultCameraState_ == ResultCameraState::Finished) {
		return;
	}

	resultCameraElapsedTime_ += deltaTime;
	const float orbitEndTheta = resultCameraStartTheta_ + kResultCameraOrbitAngle_;

	if (resultCameraState_ == ResultCameraState::Orbiting) {
		const float orbitT = std::clamp(resultCameraElapsedTime_ / kResultCameraOrbitDuration_, 0.0f, 1.0f);
		const float easedOrbitT = EaseInOut01(orbitT);
		const float theta = resultCameraStartTheta_ + kResultCameraOrbitAngle_ * easedOrbitT;
		const float distance = resultCameraStartDistance_ * (1.0f - easedOrbitT) + kResultCameraOrbitEndDistance_ * easedOrbitT;

		Game::Camera::Setter::SetCenter(cocktailPosition_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetPhiTarget(resultCameraStartPhi_, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetThetaTarget(theta, 0.0f, EaseType::IN_OUT_SINE, c_main_);
		Game::Camera::Setter::SetDistanceTarget(distance, 0.0f, EaseType::IN_OUT_SINE, c_main_);

		if (orbitT >= 1.0f) {
			resultCameraState_ = ResultCameraState::Diving;
			resultCameraElapsedTime_ = 0.0f;
		}
		return;
	}

	// 2.5周後の位置から、少し上へ膨らむ放物線で中央へ飛び込む。
	const float diveT = std::clamp(resultCameraElapsedTime_ / kResultCameraDiveDuration_, 0.0f, 1.0f);
	const float easedDiveT = EaseInOut01(diveT);
	const float horizontalDistance = kResultCameraOrbitEndDistance_ * std::cos(resultCameraStartPhi_);
	const Vector3 diveStartPosition(
	    cocktailPosition_.x + horizontalDistance * std::cos(orbitEndTheta), cocktailPosition_.y + kResultCameraOrbitEndDistance_ * std::sin(resultCameraStartPhi_),
	    cocktailPosition_.z + horizontalDistance * std::sin(orbitEndTheta));
	const Vector3 diveEndPosition(cocktailPosition_.x, cocktailPosition_.y + kResultCameraStopDistance_, cocktailPosition_.z);

	Vector3 cameraPosition = diveStartPosition * (1.0f - easedDiveT) + diveEndPosition * easedDiveT;
	cameraPosition.y += 4.0f * kResultCameraArcHeight_ * easedDiveT * (1.0f - easedDiveT);
	AimCameraFromPosition(cameraPosition, cocktailPosition_);

	if (diveT >= 1.0f) {
		resultCameraState_ = ResultCameraState::Finished;
	}
}

void ResultPhase::Update_LightModel(Model& model) {
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

void ResultPhase::Initialize_LightBuffer() {
	lightBuffer_ = {};
	lightBuffer_.ambientColor = Vector3(0.15f, 0.15f, 0.15f);

	// TitlePhaseと同じDirectional Light 1個 + Spot Light 6個
	lightBuffer_.lightCount = 7;

	Light& directionalLight = lightBuffer_.lights[0];
	directionalLight.type = 0;
	// シーン全体を強く染めすぎない、暗めのピンク色
	directionalLight.color = Vector4(0.45f, 0.12f, 0.22f, 1.0f);
	directionalLight.intensity = 0.18f;
	directionalLight.direction = Vector3(0.0f, -1.0f, 1.0f);
	directionalLight.position = Vector3(0.0f, 0.0f, 0.0f);
	directionalLight.radius = 1.0f;
	directionalLight.decay = 2.0f;
	directionalLight.distance = 10.0f;
	directionalLight.cosAngle = 0.8f;
	directionalLight.cosFalloffStart = 0.9f;

	constexpr float spotLightZPositions[] = {
	    -60.0f, -75.0f, -55.0f, -50.0f, -35.0f, -10.0f,
	};

	for (int32_t i = 0; i < 6; ++i) {
		Light& spotLight = lightBuffer_.lights[1 + i];
		spotLight.type = 2;
		spotLight.color = Vector4(1.0f, 0.35f, 0.05f, 1.0f);
		spotLight.intensity = 4.0f;
		spotLight.direction = Vector3(0.0f, -1.0f, 0.0f);
		spotLight.position = Vector3(-60.0f, 10.0f, spotLightZPositions[i]);
		spotLight.radius = 4.0f;
		spotLight.decay = 2.0f;
		spotLight.distance = 15.0f;
		spotLight.cosAngle = 0.8f;
		spotLight.cosFalloffStart = 0.9f;
	}
}

void ResultPhase::Initialize_Man() {
	manModelData_ = Game::Asset::Model::GetData(manModelID_);
	if (manModelData_ == nullptr) {
		return;
	}

	manVertexCount_ = static_cast<uint32_t>(manModelData_->vertices.size());
	manSkinInstance_ = Game::Asset::Animation::CreateSkinInstance(manModelID_);
	manResultHeapSlot_ = Game::Resource::CreateCompute(sizeof(VertexData), manVertexCount_);

	manObject_ = std::make_unique<RenderObject>();
	manObject_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModelNonIASet.VS.hlsl";
	manObject_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
	manObject_->SetupFromShaders();
	manObject_->modelID_ = manModelID_;

	manAnimationCompute_ = std::make_unique<ComputeObject>();
	manAnimationCompute_->psoConfig_.cs = "assets/shaders/Skinning/Skinning.CS.hlsl";
	manAnimationCompute_->SetupFromShaders();
	manAnimationCompute_->RegisterOutput(manResultHeapSlot_);
	manAnimationCompute_->size.x = static_cast<int32_t>((manVertexCount_ + 1023) / 1024);

	manObject_->SetCBufferData(0, ShaderType::PixelShader, &manColor_);
	manObject_->SetCBufferData(1, ShaderType::PixelShader, &manTextureID_);
	manObject_->SetSBufferData(0, ShaderType::VertexShader, Game::Resource::GetSRV(manResultHeapSlot_));

	manAnimationCompute_->SetCBufferData(0, &manVertexCount_);
	manAnimationCompute_->SetSBufferData(1, manModelData_->vertexHeapSlot);
	manAnimationCompute_->SetSBufferData(2, manModelData_->skinBindData.influenceHeapSlot);
	manAnimationCompute_->SetUAVData(0, Game::Resource::GetUAV(manResultHeapSlot_));

	// 最初はZ=-60からWalkで登場する
	manTransform_ = EulerTransforms(Vector3(14.0f, 14.0f, 14.0f), Vector3(0.0f, std::numbers::pi_v<float>, 0.0f), Vector3(-70.0f, -12.0f, kManStartZ_));
	manAnimationTime_ = 0.0f;
	manCurrentAnimationID_ = manWalkAnimationID_;
	manWinState_ = ManWinState::Walking;
	isManWalking_ = true;
	manTurnElapsedTime_ = 0.0f;
}

void ResultPhase::Update_Man() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr || manModelData_ == nullptr) {
		return;
	}

	const Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	const Matrix4x4 world = manTransform_.GetWorldMatrix();
	const Matrix4x4 wvp = world * viewProjection;

	Game::Asset::Animation::ComputeAnimationData(manCurrentAnimationID_, manSkinInstance_, manModelData_->skinBindData, manAnimationTime_);
	Game::Resource::UpdateData(manSkinInstance_.paletteHandle, manSkinInstance_.palette);

	manObject_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	manObject_->SetCBufferData(1, ShaderType::VertexShader, &world);
	manAnimationCompute_->SetSBufferData(0, Game::Resource::GetSRV(manSkinInstance_.paletteHandle));
}

void ResultPhase::Update_WinMan() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr || manModelData_ == nullptr) {
		return;
	}

	const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;

	switch (manWinState_) {
	case ManWinState::Walking:
		manCurrentAnimationID_ = manWalkAnimationID_;
		manAnimationTime_ += deltaTime;
		manTransform_.translate.z += kManWalkSpeed_ * deltaTime;

		if (manTransform_.translate.z >= kManTargetZ_) {
			manTransform_.translate.z = kManTargetZ_;

			// 到着後もWalkを続けたまま、カメラの方向へ旋回する
			manWinState_ = ManWinState::TurningToCamera;
			manCurrentAnimationID_ = manWalkAnimationID_;
			manTurnElapsedTime_ = 0.0f;
			manTurnStartY_ = manTransform_.rotate.y;

			const Vector3 cameraPosition = Game::Camera::Getter::GetWorldPosition(c_main_);
			const float directionX = cameraPosition.x - manTransform_.translate.x;
			const float directionZ = cameraPosition.z - manTransform_.translate.z;

			// このManモデルは正面が-Z側なので、求めたYawへ180度を加える
			manTurnTargetY_ = std::atan2(directionX, directionZ) + std::numbers::pi_v<float>;

			// 最短方向へ回転するよう角度差を-PI～+PIへ収める
			float angleDifference = manTurnTargetY_ - manTurnStartY_;
			while (angleDifference > std::numbers::pi_v<float>) {
				angleDifference -= std::numbers::pi_v<float> * 2.0f;
			}
			while (angleDifference < -std::numbers::pi_v<float>) {
				angleDifference += std::numbers::pi_v<float> * 2.0f;
			}
			manTurnTargetY_ = manTurnStartY_ + angleDifference;
		}
		break;

	case ManWinState::TurningToCamera: {
		// 旋回中もIdleへ切り替えず、Walkを継続する
		manCurrentAnimationID_ = manWalkAnimationID_;
		manAnimationTime_ += deltaTime;
		manTurnElapsedTime_ += deltaTime;

		float turnT = manTurnElapsedTime_ / kManTurnDuration_;
		if (turnT > 1.0f) {
			turnT = 1.0f;
		}

		// Ease In Out Sineで滑らかにカメラへ向く
		const float easedTurnT = -(std::cos(std::numbers::pi_v<float> * turnT) - 1.0f) * 0.5f;
		manTransform_.rotate.y = manTurnStartY_ + (manTurnTargetY_ - manTurnStartY_) * easedTurnT;

		if (turnT >= 1.0f) {
			manTransform_.rotate.y = manTurnTargetY_;
			isManWalking_ = false;
			manWinState_ = ManWinState::PlayingGood;
			manCurrentAnimationID_ = manGoodAnimationID_;
			manAnimationTime_ = 0.0f;
		}
		break;
	}

	case ManWinState::PlayingGood:
		manCurrentAnimationID_ = manGoodAnimationID_;
		manAnimationTime_ += deltaTime;

		if (manAnimationTime_ >= kManGoodEndTime_) {
			manAnimationTime_ = kManGoodEndTime_;
			manWinState_ = ManWinState::HoldingGood;
		}
		break;

	case ManWinState::HoldingGood:
		// Goodをループさせず、最後のポーズをそのまま保持する
		manCurrentAnimationID_ = manGoodAnimationID_;
		manAnimationTime_ = kManGoodEndTime_;
		break;
	}

	Update_Man();
}

void ResultPhase::Update_LoseMan() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr || manModelData_ == nullptr) {
		return;
	}

	const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;

	switch (manLoseState_) {
	case ManLoseState::Walking:
		// 画面左側から、中央のグラスより少し離れた位置までWalkで入る
		manCurrentAnimationID_ = manWalkAnimationID_;
		manAnimationTime_ += deltaTime;
		manTransform_.translate.z += kManWalkSpeed_ * deltaTime;

		if (manTransform_.translate.z >= kManTargetZ_) {
			manTransform_.translate.z = kManTargetZ_;
			isManWalking_ = false;
			manLoseState_ = ManLoseState::PlayingBad;
			manCurrentAnimationID_ = manBadAnimationID_;
			manAnimationTime_ = 0.0f;

			manBadStartPosition_ = manTransform_.translate;
			manBadTargetPosition_ = manBadStartPosition_;

			// Bad再生中に、現在角度から時計回りへ90度回転させる
			manBadStartRotationY_ = manTransform_.rotate.y;
			manBadTargetRotationY_ = manBadStartRotationY_ + std::numbers::pi_v<float> / 2.0f;

			// カメラの水平位置へ向かう方向を「画面手前」として、Bad中に少し近づく
			const Vector3 cameraPosition = Game::Camera::Getter::GetWorldPosition(c_main_);
			const float directionX = cameraPosition.x - manBadStartPosition_.x;
			const float directionZ = cameraPosition.z - manBadStartPosition_.z;
			const float horizontalLength = std::sqrt(directionX * directionX + directionZ * directionZ);

			if (horizontalLength > 0.0001f) {
				manBadTargetPosition_.x += directionX / horizontalLength * kManBadApproachDistance_;
				manBadTargetPosition_.z += directionZ / horizontalLength * kManBadApproachDistance_;
			}
		}
		break;

	case ManLoseState::PlayingBad: {
		manCurrentAnimationID_ = manBadAnimationID_;
		manAnimationTime_ += deltaTime;

		float badT = manAnimationTime_ / kManBadEndTime_;
		if (badT > 1.0f) {
			badT = 1.0f;
		}

		// Badの動作と同時に、イーズインアウトで少し手前へ移動・時計回りへ回転する
		const float easedBadT = -(std::cos(std::numbers::pi_v<float> * badT) - 1.0f) * 0.5f;
		manTransform_.translate.x = manBadStartPosition_.x + (manBadTargetPosition_.x - manBadStartPosition_.x) * easedBadT;
		manTransform_.translate.y = manBadStartPosition_.y + (manBadTargetPosition_.y - manBadStartPosition_.y) * easedBadT;
		manTransform_.translate.z = manBadStartPosition_.z + (manBadTargetPosition_.z - manBadStartPosition_.z) * easedBadT;
		manTransform_.rotate.y = manBadStartRotationY_ + (manBadTargetRotationY_ - manBadStartRotationY_) * easedBadT;

		if (badT >= 1.0f) {
			manTransform_.translate = manBadTargetPosition_;
			manTransform_.rotate.y = manBadTargetRotationY_;
			manAnimationTime_ = kManBadEndTime_;
			manLoseWaitElapsedTime_ = 0.0f;
			manLoseState_ = ManLoseState::WaitingAfterBad;
		}
		break;
	}

	case ManLoseState::WaitingAfterBad:
		// Badの最終姿勢を約2秒間維持する
		manCurrentAnimationID_ = manBadAnimationID_;
		manAnimationTime_ = kManBadEndTime_;
		manLoseWaitElapsedTime_ += deltaTime;

		if (manLoseWaitElapsedTime_ >= kManBadWaitDuration_) {
			manLoseState_ = ManLoseState::PlayingCatch;
			manCurrentAnimationID_ = manCatchAnimationID_;
			manAnimationTime_ = 0.0f;
		}
		break;

	case ManLoseState::PlayingCatch:
		manCurrentAnimationID_ = manCatchAnimationID_;
		manAnimationTime_ += deltaTime;

		if (manAnimationTime_ >= kManCatchEndTime_) {
			manLoseState_ = ManLoseState::PlayingThrow;
			manCurrentAnimationID_ = manThrowAnimationID_;
			manAnimationTime_ = 0.0f;
		}
		break;

	case ManLoseState::PlayingThrow:
		manCurrentAnimationID_ = manThrowAnimationID_;
		manAnimationTime_ += deltaTime;

		if (manAnimationTime_ >= kManThrowEndTime_) {
			manAnimationTime_ = kManThrowEndTime_;
			manLoseState_ = ManLoseState::HoldingThrow;
		}
		break;

	case ManLoseState::HoldingThrow:
		// Throwから別アニメーションへ切り替えず、最終姿勢で停止する
		manCurrentAnimationID_ = manThrowAnimationID_;
		manAnimationTime_ = kManThrowEndTime_;
		break;
	}

	Update_Man();
}

void ResultPhase::Draw_Man() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr) {
		return;
	}

	// 描画で使用する頂点を先にスキニングしてからManを描画する
	manAnimationCompute_->Dispatch();
	manObject_->Draw();
}

void ResultPhase::DrawImGui_Models() {
	ImGui::Begin("Result Models");

	if (ImGui::TreeNode("Bar")) {
		ImGui::DragFloat3("Scale##Bar", &barModel_.transforms_[0].scale.x, 0.01f);
		ImGui::DragFloat3("Rotate##Bar", &barModel_.transforms_[0].rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##Bar", &barModel_.transforms_[0].translate.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Signboard")) {
		ImGui::DragFloat3("Scale##Signboard", &signboardModel_.transforms_[0].scale.x, 0.01f);
		ImGui::DragFloat3("Rotate##Signboard", &signboardModel_.transforms_[0].rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##Signboard", &signboardModel_.transforms_[0].translate.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Cocktail")) {
		ImGui::DragFloat3("Scale##Cocktail", &cocktailScale_.x, 0.01f);
		ImGui::DragFloat3("Position##Cocktail", &cocktailPosition_.x, 0.1f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Man")) {
		ImGui::DragFloat3("Scale##Man", &manTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate##Man", &manTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##Man", &manTransform_.translate.x, 0.01f);
		ImGui::DragFloat("Animation Time##Man", &manAnimationTime_, 0.01f);
		ImGui::Text("Current Z : %.3f", manTransform_.translate.z);
		ImGui::Text("Animation : %s", isManWalking_ ? "Walk" : "Idle");
		ImGui::Text("Walk Animation ID : %d", manWalkAnimationID_);
		ImGui::Text("Idle Animation ID : %d", manIdleAnimationID_);
		ImGui::Text("Good Animation ID : %d", manGoodAnimationID_);
		ImGui::Text("Bad Animation ID : %d", manBadAnimationID_);
		ImGui::Text("Catch Animation ID : %d", manCatchAnimationID_);
		ImGui::Text("Throw Animation ID : %d", manThrowAnimationID_);
		ImGui::Text("Win State : %d", static_cast<int32_t>(manWinState_));
		ImGui::Text("Lose State : %d", static_cast<int32_t>(manLoseState_));
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("ResultRay")) {
		ImGui::DragFloat3("Position##ResultRay", &resultRayPosition_.x, 0.1f);
		ImGui::DragFloat("Intensity##ResultRay", &resultRayMaterialBuffer_.intensity, 0.05f, 0.0f);
		ImGui::DragFloat("Tip Radius##ResultRay", &resultRayMaterialBuffer_.tipRadius, 0.05f, 0.0f);
		ImGui::DragFloat("End Radius##ResultRay", &resultRayMaterialBuffer_.endRadius, 0.05f, 0.0f);
		ImGui::TreePop();
	}

	ImGui::End();
}

void ResultPhase::InitializeCommon() {
	// 前フェーズでTimeScaleが止められていても、
	// Resultでは移動とアニメーションが必ず進むように戻す
	Game::Time::SetTimeScale(1.0f);

	Initialize_LightModel(barModel_);
	Initialize_LightModel(signboardModel_);
	Initialize_LightModel(cocktailModel_);
	Initialize_CocktailWaterModel();
	Initialize_LightBuffer();
	Initialize_Man();
	Initialize_ResultRayModel();

	// TitlePhaseのBarと同じく、初期トランスフォームは単位行列
	barModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

	// SignboardをZ=0.0fへ配置する
	signboardModel_.transforms_[0] = EulerTransforms(Vector3(0.7f, 0.7f, 0.7f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-75.0f, 21.0f, -2.0f));

	// TitlePhaseと同じ大きさのカクテルを看板前へ配置する
	cocktailModel_.transforms_[0] = EulerTransforms(cocktailScale_, Vector3(0.0f, 0.0f, 0.0f), cocktailPosition_);
	cocktailWaterModel_.transforms_[0] = EulerTransforms(cocktailScale_, Vector3(0.0f, 0.0f, 0.0f), cocktailPosition_);
}

void ResultPhase::InitializeWin() {
	// Winは現在の暗いピンク色を維持する
	lightBuffer_.lights[0].color = Vector4(0.45f, 0.12f, 0.22f, 1.0f);
	lightBuffer_.lights[0].intensity = 0.18f;
	resultRayMaterialBuffer_.color = Vector4(0.74f, 0.28f, 0.39f, 0.10f);

	manTransform_.translate.z = kManStartZ_;
	manTransform_.rotate.y = std::numbers::pi_v<float>;
	manAnimationTime_ = 0.0f;
	manCurrentAnimationID_ = manWalkAnimationID_;
	manWinState_ = ManWinState::Walking;
	isManWalking_ = true;
	manTurnElapsedTime_ = 0.0f;
	resultRayMaterialBuffer_.reveal = 0.0f;
	resultRayRevealElapsedTime_ = 0.0f;
	isResultRayTurnedOn_ = false;
	resultCameraState_ = ResultCameraState::WaitingForInput;
	resultCameraElapsedTime_ = 0.0f;
}

void ResultPhase::InitializeLose() {
	// Loseはシーン全体のDirectional LightとResultRayを赤系へ切り替える
	lightBuffer_.lights[0].color = Vector4(0.55f, 0.03f, 0.03f, 1.0f);
	lightBuffer_.lights[0].intensity = 0.22f;
	resultRayMaterialBuffer_.color = Vector4(1.0f, 0.04f, 0.04f, 0.10f);

	// Loseでは画面左側からWalkで登場させる
	manTransform_.translate = Vector3(-70.0f, -12.0f, kManStartZ_);
	manTransform_.rotate.y = std::numbers::pi_v<float>;
	manAnimationTime_ = 0.0f;
	manCurrentAnimationID_ = manWalkAnimationID_;
	manLoseState_ = ManLoseState::Walking;
	isManWalking_ = true;
	manLoseWaitElapsedTime_ = 0.0f;
	manBadStartPosition_ = manTransform_.translate;
	manBadTargetPosition_ = manTransform_.translate;
	manBadStartRotationY_ = manTransform_.rotate.y;
	manBadTargetRotationY_ = manTransform_.rotate.y;
	resultRayMaterialBuffer_.reveal = 0.0f;
	resultRayRevealElapsedTime_ = 0.0f;
	isResultRayTurnedOn_ = false;
	resultCameraState_ = ResultCameraState::WaitingForInput;
	resultCameraElapsedTime_ = 0.0f;
}

void ResultPhase::UpdateCommon() {
	const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
	Game::Camera::Update(c_main_);
	// ヘッダーまたはImGuiで変更した位置を、モデルとカメラ中心の両方へ反映する
	cocktailModel_.transforms_[0].scale = cocktailScale_;
	cocktailModel_.transforms_[0].translate = cocktailPosition_;
	Update_LightModel(barModel_);
	Update_LightModel(signboardModel_);
	Update_LightModel(cocktailModel_);
	Update_CocktailWaterModel(deltaTime);
}

void ResultPhase::UpdateWin() {
	const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
	Update_WinMan();
	Update_ResultRayAnimation(deltaTime);
	Update_ResultCameraAnimation(deltaTime);
	Update_ResultRayModel();
}

void ResultPhase::UpdateLose() {
	const float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
	Update_LoseMan();
	Update_ResultRayAnimation(deltaTime);
	Update_ResultRayModel();
}

void ResultPhase::DrawCommon() {
	barModel_.Models_->Draw();
	signboardModel_.Models_->Draw();
	cocktailModel_.Models_->Draw();
	cocktailWaterModel_.Models_->Draw();
	Draw_Man();
	Draw_ResultRayModel();
}

void ResultPhase::DrawWin() {}

void ResultPhase::DrawLose() {}
