#include "ResultPhase.h"
#include "Game.h"

#include <cmath>
#include <numbers>

ResultPhase::ResultPhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("Result");
	Game::Camera::Setter::SetCenter(Vector3(-50.0f, 7.0f, -5.0f), 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);

	// モデル
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
	signboardModel_.ID = Game::Asset::Model::Load("assets/application/model/Signboard/Signboard.obj");
	resultRayModel_.ID = Game::Asset::Model::Load("assets/application/model/Title_Select/TitleRay.obj");

	// テクスチャ
	barModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");
	signboardModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Signboard/Signboard.png");

	// Manモデル・テクスチャ・アニメーション
	manModelID_ = Game::Asset::Model::Load("assets/application/model/Man/man.gltf");
	manTextureID_ = Game::Asset::Texture::Load("assets/application/model/Man/texture_body.png");
	manIdleAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Idle");
	manWalkAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Walk");
	manGoodAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Good");
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
	resultRayMaterialBuffer_.color = Vector4(0.74f, 0.28f, 0.39f, 0.20f);
	resultRayMaterialBuffer_.intensity = 2.5f;
	resultRayMaterialBuffer_.tipRadius = 0.4f;
	resultRayMaterialBuffer_.endRadius = 5.0f;
	resultRayMaterialBuffer_.coneLength = 16.0f;
	resultRayMaterialBuffer_.reveal = 1.0f;
	resultRayMaterialBuffer_.revealSoftness = 0.08f;
	resultRayMaterialBuffer_.density = 1.0f;
	resultRayMaterialBuffer_.centerBrightness = 1.5f;
	resultRayMaterialBuffer_.edgeSoftness = 0.25f;
	resultRayMaterialBuffer_.distanceFade = 0.35f;
	resultRayMaterialBuffer_.stepCount = 48;
	resultRayMaterialBuffer_.padding = 0.0f;
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
	if (resultRayModel_.Models_ == nullptr) {
		return;
	}

	// 半透明の光線なので、通常モデルを描いた後に重ねる
	resultRayModel_.Models_->Draw();
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
	directionalLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.intensity = 0.25f;
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
	manTransform_ = EulerTransforms(Vector3(15.0f, 15.0f, 15.0f), Vector3(0.0f, std::numbers::pi_v<float>, 0.0f), Vector3(-70.0f, -12.0f, kManStartZ_));
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
		ImGui::Text("Win State : %d", static_cast<int32_t>(manWinState_));
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
	Initialize_LightBuffer();
	Initialize_Man();
	Initialize_ResultRayModel();

	// TitlePhaseのBarと同じく、初期トランスフォームは単位行列
	barModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));

	// SignboardをZ=0.0fへ配置する
	signboardModel_.transforms_[0] = EulerTransforms(Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-75.0f, 17.0f, -5.0f));
}

void ResultPhase::InitializeWin() {
	manTransform_.translate.z = kManStartZ_;
	manTransform_.rotate.y = std::numbers::pi_v<float>;
	manAnimationTime_ = 0.0f;
	manCurrentAnimationID_ = manWalkAnimationID_;
	manWinState_ = ManWinState::Walking;
	isManWalking_ = true;
	manTurnElapsedTime_ = 0.0f;
}

void ResultPhase::InitializeLose() {
	manTransform_.translate.z = kManTargetZ_;
	manAnimationTime_ = 0.0f;
	manCurrentAnimationID_ = manIdleAnimationID_;
	isManWalking_ = false;
}

void ResultPhase::UpdateCommon() {
	Game::Camera::Update(c_main_);
	Update_LightModel(barModel_);
	Update_LightModel(signboardModel_);
	Update_ResultRayModel();
}

void ResultPhase::UpdateWin() { Update_WinMan(); }

void ResultPhase::UpdateLose() {
	manCurrentAnimationID_ = manIdleAnimationID_;
	manAnimationTime_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;
	Update_Man();
}

void ResultPhase::DrawCommon() {
	barModel_.Models_->Draw();
	signboardModel_.Models_->Draw();
	Draw_Man();
	Draw_ResultRayModel();
}

void ResultPhase::DrawWin() {}

void ResultPhase::DrawLose() {}
