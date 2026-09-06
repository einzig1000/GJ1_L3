#include "Obstacle.h"
#include"Utilities/Json/JsonManager.h"

namespace
{
    //グラス共通の変数
    float deadLine_ = 0.0f;
}

Obstacle::Obstacle()
{
    JsonManager::Load("assets/application/json/Glass/Glass.json", "/deadLine", deadLine_);

    transform_.translate.y = 1.28f;
}

Obstacle::~Obstacle()
{}

void Obstacle::Initialize()
{
    //床との当たり判定
    isHitFloor_ = false;

    //レンダーオブジェクトのインスタンス作成
    glassObj_ = std::make_unique<RenderObject>();
    glassObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    glassObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    glassObj_->SetupFromShaders();

}

void Obstacle::Update(const int32_t cameraID)
{
    //毎フレーム当たり判定を初期化する
    isHitFloor_ = false;
    Vector3 vel = { 0.0f };

    //物理を呼ぶぞ！
    if (!comCollider_.colliders.empty())
    {
        auto  phyB = comCollider_.colliders.at(0)->GetPhysicsBody();
        float mass = phyB.mass;
        vel = phyB.velocity;
    }


    if (transform_.translate.y <= deadLine_)
    {
        //床に衝突、つまり壊れる。
        isHitFloor_ = true;
    }

    //スケールタイム適用済みのデルタタイムを取得して座標を動かす
    transform_.translate += vel * Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    worldMatrix_ = transform_.GetWorldMatrix();
    Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    Matrix4x4 wvp = worldMatrix_ * viewProjection;

    glassObj_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
    glassObj_->SetCBufferData(1, ShaderType::VertexShader, &worldMatrix_);
    glassObj_->SetCBufferData(0, ShaderType::PixelShader, &color_);
    glassObj_->SetCBufferData(1, ShaderType::PixelShader, &textureID_);
}

void Obstacle::Draw()
{
    glassObj_->Draw();
}

void Obstacle::DrawImGui()
{
    ImGui::Begin("GameObj");

    if (ImGui::TreeNode("Obstacle"))
    {
        static Vector3 vel;
        ImGui::DragFloat3("velocity", &vel.x, 0.1f, -10.0f, 10.0f);
        //物理ボディ
        if (!comCollider_.colliders.empty())
        {
            auto& collider = comCollider_.colliders.at(0);
            auto  phyB = collider->GetPhysicsBody();
            float mass = phyB.mass;
            /*        ImGui::SliderFloat3("velocity", &phyB.velocity.x, -1000.0f, 1000.0f);*/
            ImGui::SliderFloat("mass", &phyB.mass, 0.001f, 1000.0f);

            collider->SetMass(phyB.mass);

            if (ImGui::Button("Shot"))
            {
                collider->SetVelocity(vel);
            }
        }

        ImGui::Checkbox("isHitFloor", &isHitFloor_);

        ImGui::DragFloat3("Scale##", &transform_.scale.x, 0.01f);
        ImGui::DragFloat3("Rotate##", &transform_.rotate.x, 0.01f);
        ImGui::DragFloat3("Translate##", &transform_.translate.x, 0.01f);

        ImGui::TreePop();
    }

    ImGui::End();
}

void Obstacle::SetGlassTypeAndLoadModels(const GlassType type)
{
	glassType_ = type;

    std::string modelPath;
    std::string texturePath;

    switch (type)
    {
    case GlassType::Bottle:
        modelPath = "assets/application/Alcohol/Bottle/Bottle.obj";
		texturePath = "assets/application/Alcohol/Bottle/Bottle.png";
        break;
    case GlassType::Champagne:
        modelPath = "assets/application/Alcohol/Champagne/Champagne.obj";
		texturePath = "assets/application/Alcohol/Champagne/Champagne.png";
        break;
    case GlassType::Gin:
		modelPath = "assets/application/Alcohol/Gin/Gin.obj";
		texturePath = "assets/application/Alcohol/Gin/Gin.png";
        break;
    case GlassType::JapaneseSake:
		modelPath = "assets/application/Alcohol/JapaneseSake/JapaneseSake.obj";
		texturePath = "assets/application/Alcohol/JapaneseSake/JapaneseSake.png";
        break;
    case GlassType::Plumwine:
		modelPath = "assets/application/Alcohol/Plumwine/Plumwine.obj";
		texturePath = "assets/application/Alcohol/Plumwine/Plumwine.png";
        break;
    case GlassType::Whiskey:
        modelPath = "assets/application/Alcohol/Whiskey/Whiskey.obj";
        texturePath = "assets/application/Alcohol/Whiskey/Whiskey.png";
        break;
    case GlassType::GLASS_MAX:
    default:
        modelPath = "assets/application/model/Cocktail/Cocktail.obj";
        texturePath = "assets/engine/texture/white1x1.png";

        break;
    }

    //モデルとテクスチャIDをセットする
    glassObj_->modelID_ = Game::Asset::Model::Load(modelPath);
    textureID_ = Game::Asset::Texture::Load(texturePath);

    comCollider_.CreateFromModelData(
        glassObj_->modelID_,
        worldMatrix_,
        CollisionTag::GetTag("Target"),
        //CollisionTag::GetTag("Table") | CollisionTag::GetTag("Glass"));
        CollisionTag::GetTag("Glass"));

    comCollider_.colliders.at(0)->SetMass(10.0f);
}
