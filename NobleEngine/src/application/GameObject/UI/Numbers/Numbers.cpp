#include "Numbers.h"

std::vector<int32_t> Numbers::modelIDs_;

namespace {
    int32_t textureID_ = -1;
}

void Numbers::Load()
{
    modelIDs_.clear();
    const std::string directoryName = "assets/application/model/Numbers/";

    for (int i = 0; i < 10; ++i) {
        int32_t modelID = Game::Asset::Model::Load(directoryName + std::to_string(i) + ".obj");
        modelIDs_.push_back(modelID);
    }

    //10を指定するとマイナス値のモデルが得られる。
    modelIDs_.push_back(Game::Asset::Model::Load("assets/application/model/Minus/minus.obj"));

    std::string textureFilePath = "assets/engine/texture/white1x1.png";
    textureID_ = Game::Asset::Texture::Load(textureFilePath);
}

Numbers::Numbers()
{
    //レンダーオブジェクトのインスタンス作成
    obj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    obj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    obj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    //obj_->psoConfig_.depthStencilID = DepthStencilID::Disable;
    obj_->SetupFromShaders();
}

Numbers::~Numbers()
{
}

void Numbers::SetStencil(const DepthStencilID id)
{
    obj_->psoConfig_.depthStencilID = id;
}

void Numbers::Initialize(const uint32_t number, const Vector3& position, const Vector3& rotation, const Vector3& scale)
{  
    //トランスフォーム
    transform_.translate = position;
    transform_.rotate = rotation;
    transform_.scale = scale;

    //モデルを取得する
    obj_->modelID_ = modelIDs_[number];
    color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
}

void Numbers::Update(const int32_t cameraID)
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

void Numbers::SetModelId(const uint32_t number)
{    //モデルを取得する
    obj_->modelID_ = modelIDs_[number];
}

void Numbers::Draw()
{
    obj_->Draw();
}

void Numbers::SetColor(const Vector4& color)
{
    color_ = color;
}
