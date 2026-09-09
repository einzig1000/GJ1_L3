#include "ResultPhase.h"
#include "Game.h"

ResultPhase::ResultPhase() {
	// カメラ
	c_main_ = Game::Camera::AddCamera("Result");
	Game::Camera::Setter::SetCenter(Vector3(-60.0f, 7.0f, -5.0f), 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetThetaTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);
	Game::Camera::Setter::SetPhiTarget(0.0f, 0.0f, EaseType::IN_OUT_SINE, c_main_);

	// モデル
	barModel_.ID = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");

	// テクスチャ
	barModel_.textureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");

	// Manモデル・テクスチャ・Idleアニメーション
	manModelID_ = Game::Asset::Model::Load("assets/application/model/Man/man.gltf");
	manTextureID_ = Game::Asset::Texture::Load("assets/application/model/Man/texture_body.png");
	manIdleAnimationID_ = Game::Asset::Animation::Load("assets/application/model/Man/man.gltf", "Idle");
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

	manTransform_ = EulerTransforms(Vector3(15.0f, 15.0f, 15.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(-70.0f, -12.0f, -5.0f));
	manAnimationTime_ = 0.0f;
}

void ResultPhase::Update_Man() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr || manModelData_ == nullptr) {
		return;
	}

	manAnimationTime_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

	const Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	const Matrix4x4 world = manTransform_.GetWorldMatrix();
	const Matrix4x4 wvp = world * viewProjection;

	Game::Asset::Animation::ComputeAnimationData(manIdleAnimationID_, manSkinInstance_, manModelData_->skinBindData, manAnimationTime_);
	Game::Resource::UpdateData(manSkinInstance_.paletteHandle, manSkinInstance_.palette);

	manObject_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	manObject_->SetCBufferData(1, ShaderType::VertexShader, &world);
	manAnimationCompute_->SetSBufferData(0, Game::Resource::GetSRV(manSkinInstance_.paletteHandle));
}

void ResultPhase::Draw_Man() {
	if (manObject_ == nullptr || manAnimationCompute_ == nullptr) {
		return;
	}

	// TestPhaseと同じ順番で登録する
	manObject_->Draw();
	manAnimationCompute_->Dispatch();
}

void ResultPhase::DrawImGui_Models() {
	ImGui::Begin("Result Models");

	if (ImGui::TreeNode("Bar")) {
		ImGui::DragFloat3("Scale##Bar", &barModel_.transforms_[0].scale.x, 0.01f);
		ImGui::DragFloat3("Rotate##Bar", &barModel_.transforms_[0].rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##Bar", &barModel_.transforms_[0].translate.x, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Man")) {
		ImGui::DragFloat3("Scale##Man", &manTransform_.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate##Man", &manTransform_.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate##Man", &manTransform_.translate.x, 0.01f);
		ImGui::DragFloat("Animation Time##Man", &manAnimationTime_, 0.01f);
		ImGui::TreePop();
	}

	ImGui::End();
}

void ResultPhase::InitializeCommon() {
	Initialize_LightModel(barModel_);
	Initialize_LightBuffer();
	Initialize_Man();

	// TitlePhaseのBarと同じく、初期トランスフォームは単位行列
	barModel_.transforms_[0] = EulerTransforms(Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));
}

void ResultPhase::InitializeWin() {}

void ResultPhase::InitializeLose() {}

void ResultPhase::UpdateCommon() {
	Game::Camera::Update(c_main_);
	Update_LightModel(barModel_);
	Update_Man();
}

void ResultPhase::UpdateWin() {}

void ResultPhase::UpdateLose() {}

void ResultPhase::DrawCommon() {
	barModel_.Models_->Draw();
	Draw_Man();
}

void ResultPhase::DrawWin() {}

void ResultPhase::DrawLose() {}
