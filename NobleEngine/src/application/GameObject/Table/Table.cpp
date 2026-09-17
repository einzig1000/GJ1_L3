#include "Table.h"

Table::Table()
{
	const std::string directory = "assets/application/model/Table/";
	const std::string filePath = directory +"Table.obj";
	const std::string textureFilePath = directory+"Table.png";

	//モデルとテクスチャIDをセットする
	modelID_ = Game::Asset::Model::Load(filePath);
	textureID_ = Game::Asset::Texture::Load(textureFilePath);

	color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
}

void Table::Initialize()
{
	//レンダーオブジェクトのインスタンス作成
	obj_ = std::make_unique<RenderObject>();
	//シンプルモデルのシェーダー適用
	obj_->psoConfig_.vs = "assets/shaders/PunctualLight/PunctualLight.VS.hlsl";
	obj_->psoConfig_.ps = "assets/shaders/PunctualLight/PunctualLight.PS.hlsl";
	obj_->SetupFromShaders();

	obj_->modelID_ = modelID_;

	//インスタンス1なので0とし行列のコンテナは考えない
	comCollider_.CreateFromModelData(
		modelID_,
		worldMatrix_,
		CollisionTag::GetTag("Table"), 
		CollisionTag::GetTag("Glass")
	);
}

void Table::Update(const int32_t cameraID)
{
	cameraPos_ = Game::Camera::Getter::GetWorldPosition(cameraID);
	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
	worldMatrix_ = transform_.GetWorldMatrix();
	wvpMatrix_ = worldMatrix_ * viewProjection;

	material_.diffuseColor = Vector3{ color_.x, color_.y, color_.z };
	material_.alpha = color_.w;
}

void Table::Draw(int32_t renderTargetID)
{
	obj_->SetCBufferData(0, ShaderType::VertexShader, &wvpMatrix_);
	obj_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix_);
	obj_->SetCBufferData(0, ShaderType::PixelShader, &cameraPos_);
	obj_->SetCBufferData(1, ShaderType::PixelShader, lightData_);
	obj_->SetCBufferData(2, ShaderType::PixelShader, &material_);
	obj_->SetCBufferData(3, ShaderType::PixelShader, &textureID_);
	obj_->Draw(renderTargetID);
}

void Table::DrawImGui()
{

	ImGui::Begin("Material");

	if (ImGui::TreeNode("Table"))
	{
		ImGui::DragFloat3("DiffuseColor", &material_.diffuseColor.x, 0.01f);
		ImGui::DragFloat3("SpecularColor", &material_.specularColor.x, 0.01f);
		ImGui::DragFloat("shininess", &material_.shininess, 0.01f);
		ImGui::DragFloat("Alpha", &material_.alpha, 0.01f);

		ImGui::TreePop();
	}

	ImGui::End();
}
