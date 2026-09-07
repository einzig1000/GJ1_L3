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
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_BACK, c_main_);

	// モデル
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
	glassModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Water/Water.obj");
	CocktailModel_.ID = Game::Asset::Model::Load("assets/application/model/Alcohol/Cocktail/Cocktail.obj");
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

void TitlePhase::Initialize_LightModels() {

	Initialize_Models(barModel_);
	Initialize_Models(glassModel_);
	Initialize_Models(CocktailModel_);
	Initialize_Models(ginModel_);
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		Initialize_Models(titleSelectModels_[i]);
	}
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Initialize_Models(iceModel_[i]);
	}

	glassModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -60.0f));
	CocktailModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -55.0f));
	ginModel_.transforms_[0] = EulerTransforms(Vector3(10.0f, 10.0f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 7.0f, -50.0f));

	// Selectになるまでは描画しないが、2つの選択モデルを先に初期化しておく
	titleSelectModels_[0].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectSelectedScale_, kTitleSelectSelectedScale_, kTitleSelectSelectedScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 15.0f, -60.0f));
	titleSelectModels_[1].transforms_[0] =
	    EulerTransforms(Vector3(kTitleSelectNormalScale_, kTitleSelectNormalScale_, kTitleSelectNormalScale_), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 15.0f, -50.0f));

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

	preLiftElapsedTime_ = 0.0f;
	isPreLiftFinished_ = false;
	glassTiltElapsedTime_ = 0.0f;
	isGlassTiltFinished_ = false;
	glassTiltHoldElapsedTime_ = 0.0f;
	isGlassTiltHoldFinished_ = false;
	iceAnimationElapsedTime_ = 0.0f;
	isIceAnimationFinished_ = false;
	glassReturnElapsedTime_ = 0.0f;
	isGlassReturnFinished_ = false;
	ginAnimationElapsedTime_ = 0.0f;
	titlePhaseSelection_ = TitlePhaseSelection::Start;
	selectedTitleIndex_ = 0;
	previousAnimationTime_ = std::chrono::steady_clock::now();
}

void TitlePhase::Update_Animation() {
	// ジンのアニメーションまで完了した後は更新しない
	if (titlePhaseSelection_ == TitlePhaseSelection::Select) {
		return;
	}

	const std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();

	float deltaTime = std::chrono::duration<float>(currentTime - previousAnimationTime_).count();

	previousAnimationTime_ = currentTime;

	// デバッグ停止などで極端に大きな時間が入った場合の瞬間移動を防ぐ
	if (deltaTime > 0.1f) {
		deltaTime = 0.1f;
	}

	// ========================================
	// Glass And Ice Pre-Lift Animation
	// ========================================

	if (!isPreLiftFinished_) {
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

		// 持ち上げが終わってから氷の移動を開始する
		return;
	}

	// ========================================
	// Glass And Ice Tilt Animation
	// ========================================

	if (!isGlassTiltFinished_) {
		glassTiltElapsedTime_ += deltaTime;

		float tiltT = glassTiltElapsedTime_ / kGlassTiltDuration_;
		if (tiltT >= 1.0f) {
			tiltT = 1.0f;
			isGlassTiltFinished_ = true;
		}

		const float currentTiltAngle = kGlassTiltAngle_ * tiltT;
		const float tiltCos = std::cos(currentTiltAngle);
		const float tiltSin = std::sin(currentTiltAngle);
		const float glassPivotY = 7.0f + kGlassLiftHeight_;
		const float glassPivotZ = kIceStartZ_;

		const Vector3 iceScale(0.2f, 0.2f, 0.2f);
		const Vector3 iceRotate(currentTiltAngle, 0.0f, 0.0f);

		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			const float liftedIceY = kIceStartBottomY_ + static_cast<float>(i) * kIceVerticalSpacing_ + kGlassLiftHeight_;
			const float relativeY = liftedIceY - glassPivotY;

			// グラスの基準位置を中心として、氷の位置もグラスと同じ角度だけ回転する
			const float tiltedY = glassPivotY + relativeY * tiltCos;
			const float tiltedZ = glassPivotZ + relativeY * tiltSin;
			const Vector3 tiltedPosition(kIceStartX_, tiltedY, tiltedZ);

			iceTiltedStartPositions_[i] = tiltedPosition;
			iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, tiltedPosition);
		}

		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(currentTiltAngle, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, glassPivotY, -60.0f);
		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);

		return;
	}

	// ========================================
	// Tilt Hold
	// ========================================

	if (!isGlassTiltHoldFinished_) {
		glassTiltHoldElapsedTime_ += deltaTime;

		if (glassTiltHoldElapsedTime_ >= kGlassTiltHoldDuration_) {
			glassTiltHoldElapsedTime_ = kGlassTiltHoldDuration_;
			isGlassTiltHoldFinished_ = true;
		}

		// 傾いた状態を維持してから、氷の移動へ進む
		return;
	}

	if (!isIceAnimationFinished_) {
		iceAnimationElapsedTime_ += deltaTime;

		float iceT = iceAnimationElapsedTime_ / kIceMoveDuration_;
		if (iceT >= 1.0f) {
			iceT = 1.0f;
			isIceAnimationFinished_ = true;
		}

		const Vector3 iceScale(0.2f, 0.2f, 0.2f);

		// カクテルへ到着した時点で、氷本体も反時計回りへ90度になる
		const float targetIceRotationY = -std::numbers::pi_v<float> * 0.5f;
		const Vector3 iceRotate(kGlassTiltAngle_ * (1.0f - iceT), targetIceRotationY * iceT, 0.0f);

		for (int32_t i = 0; i < kMaxIceCount_; ++i) {
			// グラスと一緒に傾いた位置から、それぞれの完成位置へ向かう
			const Vector3 linearPosition = iceTiltedStartPositions_[i] * (1.0f - iceT) + iceTargetPositions_[i] * iceT;

			// t=0とt=1では高さ0、中央のt=0.5で最大になる放物線
			const float arcOffsetY = 4.0f * kIceArcHeight_ * iceT * (1.0f - iceT);
			const Vector3 position = linearPosition + Vector3(0.0f, arcOffsetY, 0.0f);

			iceModel_[i].transforms_[0] = EulerTransforms(iceScale, iceRotate, position);
		}

		// 氷が移動している間も、グラスは持ち上げて傾けた状態を維持する
		const Vector3 glassScale(10.0f, 10.0f, 10.0f);
		const Vector3 glassRotate(kGlassTiltAngle_, 0.0f, 0.0f);
		const Vector3 glassPosition(-60.0f, 7.0f + kGlassLiftHeight_, -60.0f);

		glassModel_.transforms_[0] = EulerTransforms(glassScale, glassRotate, glassPosition);

		// 氷が移動中なら、まだジンのアニメーションへ進まない
		if (!isIceAnimationFinished_) {
			return;
		}
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

	ginAnimationElapsedTime_ += deltaTime;

	float ginT = ginAnimationElapsedTime_ / kGinTiltDuration_;
	if (ginT >= 1.0f) {
		ginT = 1.0f;
	}

	const float ginTiltAmount = std::sin(std::numbers::pi_v<float> * ginT);

	const Vector3 ginScale(10.0f, 10.0f, 10.0f);
	const Vector3 ginRotate(-std::numbers::pi_v<float> / 3.0f * ginTiltAmount, 0.0f, 0.0f);
	const Vector3 ginPosition(-60.0f, 7.0f, -50.0f);

	ginModel_.transforms_[0] = EulerTransforms(ginScale, ginRotate, ginPosition);

	// ジンが元の姿勢へ戻ったらタイトル選択状態をSelectへ変更
	if (ginT >= 1.0f) {
		titlePhaseSelection_ = TitlePhaseSelection::Select;
	}
}

void TitlePhase::Update_LightModels() {
	Update_Model(barModel_);
	Update_Model(glassModel_);
	Update_Model(CocktailModel_);
	Update_Model(ginModel_);
	for (int32_t i = 0; i < kMaxIceCount_; ++i) {
		Update_Model(iceModel_[i]);
	}
	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		Update_Model(titleSelectModels_[i]);
	}
}

void TitlePhase::Update_TitleSelect() {
	if (titlePhaseSelection_ != TitlePhaseSelection::Select) {
		return;
	}

	// Aキーまたは左矢印キーでZ=-60側を選択
	if (Game::IO::Key::IsJustPressed('A') || Game::IO::Key::IsJustPressed(0x25)) {
		selectedTitleIndex_ = 0;
	}

	// Dキーまたは右矢印キーでZ=-50側を選択
	if (Game::IO::Key::IsJustPressed('D') || Game::IO::Key::IsJustPressed(0x27)) {
		selectedTitleIndex_ = 1;
	}

	for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
		const float scale = i == selectedTitleIndex_ ? kTitleSelectSelectedScale_ : kTitleSelectNormalScale_;
		const float z = i == 0 ? -60.0f : -50.0f;

		titleSelectModels_[i].transforms_[0] = EulerTransforms(Vector3(scale, scale, scale), Vector3(0.0f, 0.0f, 0.0f), Vector3(-60.0f, 15.0f, z));
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

	// 2つの選択モデルはSelect中だけ表示する
	if (titlePhaseSelection_ == TitlePhaseSelection::Select) {
		for (int32_t i = 0; i < kTitleSelectCount_; ++i) {
			titleSelectModels_[i].Models_->Draw();
		}
	}
}
