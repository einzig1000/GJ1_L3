#include "TestPhase.h"
#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <externals/MagicEnum/magic_enum.hpp>


TestPhase::TestPhase()
{
	// カメラ
	c_main_ = Game::Camera::AddCamera("SimpleModels");
	// モデル
	m_cube_ = Game::Asset::Model::Load("assets/engine/model/cube/cube.obj");
	m_plane_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
	m_human_ = Game::Asset::Model::Load("assets/engine/model/human/sneakWalk.gltf");
	m_gentleman_ = Game::Asset::Model::Load("assets/application/model/gentleman/gentleman.gltf");
	// テクスチャ
	t_monsterBall_ = Game::Asset::Texture::Load("assets/engine/texture/monsterBall.png");
	t_uvChecker_ = Game::Asset::Texture::Load("assets/engine/texture/uvChecker.png");
	// アニメーション
	a_sneakWalk_ = Game::Asset::Animation::Load("assets/engine/model/human/sneakWalk.gltf", "sneakWalk");
}

TestPhase::~TestPhase()
{}

void TestPhase::Initialize()
{
	// フェーズ初期化
	nextPhase_ = Phase::Phase_None;

	Initialize_SimpleModels();
	Initialize_Sprite();
	Initialize_Animation();
}


void TestPhase::Update()
{
	Game::Camera::Update(c_main_);

	Update_SimpleModels();
	Update_Sprite();
	Update_Animation();
}


void TestPhase::Draw()
{
	//Draw_SimpleModels();
	Draw_Sprite();
	Draw_Animation();
}


void TestPhase::DrawImGui()
{
	DrawImGui_SimpleModels();
	DrawImGui_Sprite();
	DrawImGui_Animation();
}

void TestPhase::Initialize_SimpleModels()
{
	simpleModels_ = std::make_unique<RenderObject>();
	simpleModels_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModels.VS.hlsl";
	simpleModels_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModels.PS.hlsl";
	simpleModels_->SetupFromShaders();
	simpleModels_->modelID_ = m_cube_;
	simpleModels_->instanceNum_ = instanceCount_;

	worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();
	colorHeapSlot_ = Game::Resource::CreateDynamic();
	textureIndexHeapSlot_ = Game::Resource::CreateDynamic();

	transforms_.resize(instanceCount_, EulerTransforms());
	worldMatrices_.resize(instanceCount_, Matrix4x4());
	colors_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	textureIndices_.resize(instanceCount_, t_uvChecker_);
}

void TestPhase::Update_SimpleModels()
{
	for (int i = 0; i < instanceCount_; i++)
	{
		worldMatrices_[i] = transforms_[i].GetWorldMatrix();
	}

	Game::Resource::UpdateData(worldMatrixHeapSlot_, worldMatrices_);
	Game::Resource::UpdateData(colorHeapSlot_, colors_);
	Game::Resource::UpdateData(textureIndexHeapSlot_, textureIndices_);

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlot_);
	Vector2uint psHeapSlot{ Game::Resource::GetSRV(colorHeapSlot_), Game::Resource::GetSRV(textureIndexHeapSlot_) };

	simpleModels_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);
	simpleModels_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);
	simpleModels_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);
}

void TestPhase::Draw_SimpleModels()
{
	simpleModels_->Draw();
}

void TestPhase::DrawImGui_SimpleModels()
{
	ImGui::Begin("SimpleModelsTest");

	for (int i = 0; i < instanceCount_; i++)
	{
		if (ImGui::TreeNode(("Instance " + std::to_string(i)).c_str()))
		{
			ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transforms_[i].scale.x, 0.01f);
			ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transforms_[i].rotate.x, 0.01f);
			ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transforms_[i].translate.x, 0.01f);
			ImGui::ColorEdit4(("Color##" + std::to_string(i)).c_str(), &colors_[i].x);
			const char* textureNames[] = { "uvChecker", "monsterBall" };
			int currentTextureIndex = textureIndices_[i];
			if (ImGui::Combo(("Texture##" + std::to_string(i)).c_str(), &currentTextureIndex, textureNames, IM_ARRAYSIZE(textureNames)))
			{
				textureIndices_[i] = currentTextureIndex == 0 ? t_uvChecker_ : t_monsterBall_;
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}


void TestPhase::Initialize_Sprite()
{
	sprite_ = std::make_unique<RenderObject>();
	sprite_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
	sprite_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
	sprite_->SetupFromShaders();
	sprite_->modelID_ = m_plane_;
	spriteTexture_ = t_uvChecker_;

	spriteTransform_.scale = Vector3(128.0f, 128.0f, 0.0f);
	spriteTransform_.translate = Vector3(140.0f, 140.0f, 0.0f);
}

void TestPhase::Update_Sprite()
{
	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	Matrix4x4 orthographicMatrix = Game::Camera::Getter::GetOrthoProjectionMatrix(c_main_);
	Matrix4x4 worldMatrix = spriteTransform_.GetWorldMatrix();
	Matrix4x4 wvp = worldMatrix * orthographicMatrix;
	Vector4 color = spriteColor_;

	sprite_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	sprite_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix);
	sprite_->SetCBufferData(0, ShaderType::PixelShader, &color);
	sprite_->SetCBufferData(1, ShaderType::PixelShader, &spriteTexture_);
}

void TestPhase::Draw_Sprite()
{
	sprite_->Draw();
}

void TestPhase::DrawImGui_Sprite()
{
	ImGui::Begin("SpriteTest");
	ImGui::DragFloat3("Scale", &spriteTransform_.scale.x, 0.01f);
	ImGui::DragFloat3("Rotate", &spriteTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Translate", &spriteTransform_.translate.x, 0.01f);
	ImGui::ColorEdit4("Color", &spriteColor_.x);
	const char* textureNames[] = { "uvChecker", "monsterBall" };
	int currentTextureIndex = (spriteTexture_ == t_uvChecker_) ? 0 : 1;
	if (ImGui::Combo("Texture", &currentTextureIndex, textureNames, IM_ARRAYSIZE(textureNames)))
	{
		spriteTexture_ = (currentTextureIndex == 0) ? t_uvChecker_ : t_monsterBall_;
	}
	ImGui::End();
}


void TestPhase::Initialize_Animation()
{
	modelData_ = Game::Asset::Model::GetData(m_human_);
	numVertices_ = static_cast<uint32_t>(modelData_->vertices.size());
	skinInstance_ = Game::Asset::Animation::CreateSkinInstance(m_human_);
	resultHeapSlot_ = Game::Resource::CreateCompute(sizeof(VertexData), numVertices_);


	animation_ = std::make_unique<RenderObject>();
	animation_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModelNonIASet.VS.hlsl";
	animation_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
	animation_->SetupFromShaders();
	animation_->modelID_ = m_human_;

	animationCompute_ = std::make_unique<ComputeObject>();
	animationCompute_->psoConfig_.cs = "assets/shaders/Skinning/Skinning.CS.hlsl";
	animationCompute_->SetupFromShaders();
	animationCompute_->RegisterOutput(resultHeapSlot_);
	animationCompute_->size.x = int32_t((numVertices_ + 1023) / 1024);


	const Vector4    color = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Matrix4x4  world = Matrix4x4::MakeIdentity4x4();
	animation_->SetCBufferData(0, ShaderType::PixelShader, &color);
	animation_->SetCBufferData(1, ShaderType::PixelShader, &t_uvChecker_);
	animation_->SetSBufferData(0, ShaderType::VertexShader, Game::Resource::GetSRV(resultHeapSlot_));

	animationCompute_->SetCBufferData(0, &numVertices_);
	animationCompute_->SetSBufferData(1, modelData_->vertexHeapSlot);
	animationCompute_->SetSBufferData(2, modelData_->skinBindData.influenceHeapSlot);
	animationCompute_->SetUAVData(0, Game::Resource::GetUAV(resultHeapSlot_));
}

void TestPhase::Update_Animation()
{
	animationTime_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	Matrix4x4 world = Matrix4x4::MakeIdentity4x4();
	Matrix4x4 wvp = world * viewProjection;

	Game::Asset::Animation::ComputeAnimationData(a_sneakWalk_, skinInstance_, modelData_->skinBindData, animationTime_);
	Game::Resource::UpdateData(skinInstance_.paletteHandle, skinInstance_.palette);

	animation_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	animation_->SetCBufferData(1, ShaderType::VertexShader, &world);

	animationCompute_->SetSBufferData(0, Game::Resource::GetSRV(skinInstance_.paletteHandle));
}



void TestPhase::Draw_Animation()
{
	animation_->Draw();
	animationCompute_->Dispatch();
}

void TestPhase::DrawImGui_Animation()
{
	ImGui::Begin("AnimationTest");
	ImGui::DragFloat("AnimationTime", &animationTime_, 0.01f);
	ImGui::End();
}
