#include "HumanModel.h"

HumanModel::HumanModel()
{
    std::string directory = "assets/application/model/Woman/";
    std::string filePath = directory+"woman.gltf";
    // モデル
    model_ = Game::Asset::Model::Load(filePath);
    // アニメーション
    animationIDs_["Idle"] = Game::Asset::Animation::Load(filePath, "Idle");
    animationIDs_["Walk"] = Game::Asset::Animation::Load(filePath, "Walk");
    // テクスチャ
    texture_ = Game::Asset::Texture::Load(directory + "texture_body.png");
}

HumanModel::~HumanModel()
{
}

void HumanModel::Initialize()
{
	modelData_ = Game::Asset::Model::GetData(model_);
	numVertices_ = static_cast<uint32_t>(modelData_->vertices.size());
	skinInstance_ = Game::Asset::Animation::CreateSkinInstance(model_);
	resultHeapSlot_ = Game::Resource::CreateCompute(sizeof(VertexData), numVertices_);

	animation_ = std::make_unique<RenderObject>();
	animation_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModelNonIASet.VS.hlsl";
	animation_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
	animation_->SetupFromShaders();
	animation_->modelID_ = model_;

	animationCompute_ = std::make_unique<ComputeObject>();
	animationCompute_->psoConfig_.cs = "assets/shaders/Skinning/Skinning.CS.hlsl";
	animationCompute_->SetupFromShaders();
	animationCompute_->RegisterOutput(resultHeapSlot_);
	animationCompute_->size.x = int32_t((numVertices_ + 1023) / 1024);

	const Vector4    color = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Matrix4x4  world = Matrix4x4::MakeIdentity4x4();
	animation_->SetCBufferData(0, ShaderType::PixelShader, &color);
	animation_->SetCBufferData(1, ShaderType::PixelShader, &texture_);
	animation_->SetSBufferData(0, ShaderType::VertexShader, Game::Resource::GetSRV(resultHeapSlot_));

	animationCompute_->SetCBufferData(0, &numVertices_);
	animationCompute_->SetSBufferData(1, modelData_->vertexHeapSlot);
	animationCompute_->SetSBufferData(2, modelData_->skinBindData.influenceHeapSlot);
	animationCompute_->SetUAVData(0, Game::Resource::GetUAV(resultHeapSlot_));

	currentAnimationName_ = "Idle";


	transform_ = { .scale = {1.0f,1.0f,1.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} };
	worldMatrix_ = transform_.GetWorldMatrix();
	//インスタンス1なので0とし行列のコンテナは考えない
	comCollider_.CreateFromModelData(model_, worldMatrix_, CollisionTag::GetTag("Target"), CollisionTag::GetTag("Glass")| CollisionTag::GetTag("Table"));

}

void HumanModel::Update(const int32_t cameraID)
{
	animationTime_ += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

	Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);

	
	worldMatrix_ = transform_.GetWorldMatrix();
	Matrix4x4 wvp = worldMatrix_ * viewProjection;

	Game::Asset::Animation::ComputeAnimationData(animationIDs_[currentAnimationName_], skinInstance_, modelData_->skinBindData, animationTime_);
	Game::Resource::UpdateData(skinInstance_.paletteHandle, skinInstance_.palette);

	animation_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
	animation_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix_);

	animationCompute_->SetSBufferData(0, Game::Resource::GetSRV(skinInstance_.paletteHandle));
}

void HumanModel::Draw()
{
	animation_->Draw();
	animationCompute_->Dispatch();
}

void HumanModel::DrawImGui()
{

	ImGui::Begin("GameObj");

	if (ImGui::TreeNode("Human")) {

		static int32_t currentAnimID = 0; // 現在選択中の animationID

		if (ImGui::BeginCombo("Animation", currentAnimationName_.c_str()))
		{
			for (auto& [label, animID] : animationIDs_)
			{
				bool isSelected = (currentAnimID == animID);

				if (ImGui::Selectable(label.c_str(), isSelected))
				{
					currentAnimID = animID;
					currentAnimationName_ = label;
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::DragFloat("AnimationTime", &animationTime_, 0.01f);

			if (ImGui::TreeNode("Transform"))
			{
				ImGui::DragFloat3("Scale##", &transform_.scale.x, 0.01f);
				ImGui::DragFloat3("Rotate##" , &transform_.rotate.x, 0.01f);
				ImGui::DragFloat3("Translate##" ,&transform_.translate.x, 0.01f);
				ImGui::TreePop();
			}
		
		ImGui::TreePop();
	}

	ImGui::End();
}

