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

void UIModel::Initialize(const int32_t modelID, const int32_t textureID,const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
    //トランスフォーム
    transform_.translate = position;
    transform_.rotate = rotation;
    transform_.scale = scale;
    
    textureID_ = textureID;

    //モデルを取得する
    obj_->modelID_ = modelID;

    color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
}

void UIModel::Update(const int32_t cameraID)
{
    worldMatrix_ = transform_.GetWorldMatrix();

    Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    Matrix4x4 orthographicMatrix = Game::Camera::Getter::GetOrthoProjectionMatrix(cameraID);
    Matrix4x4 wvp = worldMatrix_ * orthographicMatrix;

    obj_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
    obj_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix_);
    obj_->SetCBufferData(0, ShaderType::PixelShader, &color_);
    obj_->SetCBufferData(1, ShaderType::PixelShader, &textureID_);
}

void UIModel::Draw()
{
    obj_->Draw();
}
