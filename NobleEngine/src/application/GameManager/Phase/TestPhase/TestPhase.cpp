#include "TestPhase.h"
#include <Utilities/Json/JsonManager.h>
#include <Utilities/functions.h>
#include <externals/MagicEnum/magic_enum.hpp>


TestPhase::TestPhase()
{
	c_main_ = Game::Camera::AddCamera("main");
	m_cube_ = Game::Asset::Model::Load("assets/engine/model/cube/cube.obj"); 
	t_uvChecker_ = Game::Asset::Texture::Load("assets/engine/texture/uvChecker.png");
	t_monsterBall_ = Game::Asset::Texture::Load("assets/engine/texture/monsterBall.png");

	worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();
	colorHeapSlot_ = Game::Resource::CreateDynamic();
	textureIndexHeapSlot_ = Game::Resource::CreateDynamic();

	renderObject_ = std::make_unique<RenderObject>();
	renderObject_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModels.VS.hlsl";
	renderObject_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModels.PS.hlsl";
	renderObject_->SetupFromShaders();
	renderObject_->modelID_ = m_cube_;
	renderObject_->instanceNum_ = instanceCount_;

	transforms_.resize(instanceCount_, EulerTransforms());
	worldMatrices_.resize(instanceCount_, Matrix4x4());
	colors_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	textureIndices_.resize(instanceCount_, t_uvChecker_);
}

TestPhase::~TestPhase()
{}

void TestPhase::Initialize()
{
	// フェーズ初期化
	nextPhase_ = Phase::Phase_None;
}


void TestPhase::Update()
{
	Game::Camera::Update(c_main_);

	for (int i = 0; i < instanceCount_; i++)
	{
		worldMatrices_[i] = transforms_[i].GetWorldMatrix();
	}

	Game::Resource::UpdateData(worldMatrixHeapSlot_, worldMatrices_.data(), sizeof(Matrix4x4), worldMatrices_.size());
	Game::Resource::UpdateData(colorHeapSlot_, colors_.data(), sizeof(Vector4), colors_.size());
	Game::Resource::UpdateData(textureIndexHeapSlot_, textureIndices_.data(), sizeof(int32_t), textureIndices_.size());

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
	int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlot_);
	Vector2int psHeapSlot{ Game::Resource::GetSRV(colorHeapSlot_), Game::Resource::GetSRV(textureIndexHeapSlot_) };

	renderObject_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);
	renderObject_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);
	renderObject_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);
}


void TestPhase::Draw()
{
	renderObject_->Draw();
}


void TestPhase::DrawImGui()
{
	ImGui::Begin("TestPhase");

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
