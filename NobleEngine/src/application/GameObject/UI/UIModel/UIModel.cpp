#include "UIModel.h"

UIModel::UIModel()
{    //レンダーオブジェクトのインスタンス作成
    obj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    obj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    obj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    /*obj_->psoConfig_.depthStencilID = DepthStencilID::Disable;*/

    obj_->SetupFromShaders();


}

UIModel::~UIModel()
{
}

void UIModel::SetStencil(const DepthStencilID id)
{
    obj_->psoConfig_.depthStencilID = id;
}

void UIModel::Initialize(const int32_t modelID, const int32_t textureID, const EulerTransforms& transform, Matrix4x4* parent)
{
    //トランスフォーム
    transform_ = transform;
    textureID_ = textureID;

    //モデルを取得する
    obj_->modelID_ = modelID;

    color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
    parent_ = parent;
}

void UIModel::Update(const int32_t cameraID)
{
    if (parent_) {
        Matrix4x4 child =  transform_.GetWorldMatrix();
        worldMatrix_ = child * *parent_;
    } else {
        worldMatrix_ = transform_.GetWorldMatrix();
    }
      Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    Matrix4x4 wvp = worldMatrix_ * viewProjection;

    obj_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
    obj_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix_);
    obj_->SetCBufferData(0, ShaderType::PixelShader, &color_);
    obj_->SetCBufferData(1, ShaderType::PixelShader, &textureID_);
}

void UIModel::Draw(const int32_t renderTexture)
{
    obj_->Draw(renderTexture);
}

void UIModel::DebugUI(const int id)
{

    ImGui::Begin("UI");

    ImGui::PushID(id);
    if (ImGui::TreeNode("UIModel")) {

        ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
        ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
        ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
        ImGui::ColorEdit4("Color", &color_.x);
        ImGui::TreePop();
    }

    ImGui::PopID();

    ImGui::End();

}

void UIModel::SetRotateX(const float rotate)
{
    transform_.rotate.x = rotate;
}
