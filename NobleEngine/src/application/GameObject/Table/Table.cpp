#include "Table.h"

Table::Table()
{
	const std::string directory = "assets/application/model/Table/";
	const std::string filePath = directory +"Table.obj";
	const std::string textureFilePath = directory+"Table.png";

	//モデルとテクスチャIDをセットする
	modelID_ = Game::Asset::Model::Load(filePath);
	textureID_ = Game::Asset::Texture::Load(textureFilePath);
}

void Table::Initialize()
{
	//レンダーオブジェクトのインスタンス作成
	obj_ = std::make_unique<RenderObject>();
	//シンプルモデルのシェーダー適用
	obj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModels.VS.hlsl";
	obj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModels.PS.hlsl";
	obj_->SetupFromShaders();

	obj_->modelID_ = modelID_;
	obj_->instanceNum_ = instanceCount_;

	worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();
	colorHeapSlot_ = Game::Resource::CreateDynamic();
	textureIndexHeapSlot_ = Game::Resource::CreateDynamic();

	transforms_.resize(instanceCount_, EulerTransforms());
	worldMatrices_.resize(instanceCount_, Matrix4x4());

	colors_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	textureIndices_.resize(instanceCount_, textureID_);
}

void Table::Update(const int32_t cameraID)
{
	for (int i = 0; i < instanceCount_; i++)
	{
		worldMatrices_[i] = transforms_[i].GetWorldMatrix();
	}

	Game::Resource::UpdateData(worldMatrixHeapSlot_, worldMatrices_);
	Game::Resource::UpdateData(colorHeapSlot_, colors_);
	Game::Resource::UpdateData(textureIndexHeapSlot_, textureIndices_);

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
	int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlot_);
	Vector2uint psHeapSlot{ Game::Resource::GetSRV(colorHeapSlot_), Game::Resource::GetSRV(textureIndexHeapSlot_) };

	obj_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);
	obj_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);
	obj_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);
}

void Table::Draw()
{
	obj_->Draw();
}

void Table::DrawImGui()
{
	ImGui::Begin("GameObj");

	if (ImGui::TreeNode("Table")) {
		for (int i = 0; i < instanceCount_; i++)
		{
			if (ImGui::TreeNode(("Instance " + std::to_string(i)).c_str()))
			{
				ImGui::DragFloat3(("Scale##" + std::to_string(i)).c_str(), &transforms_[i].scale.x, 0.01f);
				ImGui::DragFloat3(("Rotate##" + std::to_string(i)).c_str(), &transforms_[i].rotate.x, 0.01f);
				ImGui::DragFloat3(("Translate##" + std::to_string(i)).c_str(), &transforms_[i].translate.x, 0.01f);
				ImGui::ColorEdit4(("Color##" + std::to_string(i)).c_str(), &colors_[i].x);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}


	ImGui::End();
}
