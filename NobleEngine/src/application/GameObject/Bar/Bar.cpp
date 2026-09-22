#include "Bar.h"

Bar::Bar()
{
    bar_ = std::make_unique<RenderObject>();
    bar_->psoConfig_.vs = "assets/shaders/PunctualLight/PunctualLight.VS.hlsl";
    bar_->psoConfig_.ps = "assets/shaders/PunctualLight/PunctualLight.PS.hlsl";
    bar_->modelID_ = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
    bar_->SetupFromShaders();
    barTextureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");

    barTransforms_.scale = Vector3{ 0.1f, 0.1f, 0.1f };
    barTransforms_.translate = Vector3{ -1.0f, 0.6f, 9.0f };
}

Bar::~Bar()
{
}

void Bar::Initialize()
{


}

void Bar::Update(const int cameraID)
{
    const Matrix4x4 viewPro = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    const Vector3 cameraPos = Game::Camera::Getter::GetWorldPosition(cameraID);

    Matrix4x4 world = barTransforms_.GetWorldMatrix();
    Matrix4x4 wvp = world * viewPro;
    Vector4 color = Vector4{ 1.0f, 0.0f, 0.0f, 1.0f };

    bar_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
    bar_->SetCBufferData(1, ShaderType::VertexShader, &world);
    bar_->SetCBufferData(0, ShaderType::PixelShader, &cameraPos);
    bar_->SetCBufferData(1, ShaderType::PixelShader, lightData_);
    bar_->SetCBufferData(2, ShaderType::PixelShader, &barMaterial_);
    bar_->SetCBufferData(3, ShaderType::PixelShader, &barTextureID_);
}

void Bar::Draw(const int renderTexture3DId)
{

   bar_->Draw(renderTexture3DId);
    
}

void Bar::DrawImGui()
{   

    ImGui::Begin("Editor");

    // バーの位置
    if (ImGui::TreeNode("Bar"))
    {
        ImGui::DragFloat3("Scale", &barTransforms_.scale.x, 0.01f);
        ImGui::DragFloat3("Translate", &barTransforms_.translate.x, 1.0f);

        ImGui::TreePop();
    }

    ImGui::End();
}
