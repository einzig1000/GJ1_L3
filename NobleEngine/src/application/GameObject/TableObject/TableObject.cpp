#include "TableObject.h"
#include"Utilities/Json/JsonManager.h"

namespace
{
    //グラス共通の変数
    float deadLine_ = 0.0f;
}

TableObject::TableObject()
{
    JsonManager::Load("assets/application/json/Glass/Glass.json", "/deadLine", deadLine_);

    //レンダーオブジェクトのインスタンス作成
    glassObj_ = std::make_unique<RenderObject>();
    glassObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    glassObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    glassObj_->SetupFromShaders();
}

TableObject::~TableObject()
{}

void TableObject::Initialize()
{
    //床との当たり判定
    isHitFloor_ = false;

    transform_.translate.y = 1.28f;
}

void TableObject::Update(const int32_t cameraID)
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

void TableObject::Draw()
{
    glassObj_->Draw();
}

void TableObject::DrawImGui()
{
    ImGui::Begin("GameObj");

    if (ImGui::TreeNode("TableObject"))
    {
        static Vector3 vel;
        ImGui::DragFloat3("velocity", &vel.x, 0.1f, -10.0f, 10.0f);
        //物理ボディ
        if (!comCollider_.colliders.empty())
        {
            auto& collider = comCollider_.colliders.at(0);
            auto  phyB = collider->GetPhysicsBody();

            ImGui::SliderFloat("mass", &phyB.mass, 0.001f, 1000.0f);
            ImGui::SliderFloat("coefficiendOfRestituion", &phyB.coefficiendOfRestituion, 0.001f, 1000.0f);

            collider->SetMass(phyB.mass);
            collider->SetCoefficiendOfRestituion(phyB.coefficiendOfRestituion);


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

void TableObject::SetGlassTypeAndLoadModels(const GlassType type)
{
	glassType_ = type;

    std::string modelPath;
    std::string texturePath;
	uint32_t myColliderTag = 0;
	uint32_t targetColliderTag = 0;
	float mass = 0.0f;

    switch (type)
    {
    case GlassType::Glass:
        modelPath = "assets/application/model/Cocktail/Cocktail.obj";
        texturePath = "assets/engine/texture/white1x1.png";
		myColliderTag = CollisionTag::GetTag("Glass");
        targetColliderTag = (CollisionTag::GetTag("Target") | CollisionTag::GetTag("Obstacles"));
		mass = 1.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 0.5f };
        break;
    case GlassType::Bottle:
        modelPath = "assets/application/Alcohol/Bottle/Bottle.obj";
		texturePath = "assets/application/Alcohol/Bottle/Bottle.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		mass = 10.0f;
        break;
    case GlassType::Champagne:
        modelPath = "assets/application/Alcohol/Champagne/Champagne.obj";
		texturePath = "assets/application/Alcohol/Champagne/Champagne.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case GlassType::Gin:
		modelPath = "assets/application/Alcohol/Gin/Gin.obj";
		texturePath = "assets/application/Alcohol/Gin/Gin.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case GlassType::JapaneseSake:
		modelPath = "assets/application/Alcohol/JapaneseSake/JapaneseSake.obj";
		texturePath = "assets/application/Alcohol/JapaneseSake/JapaneseSake.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case GlassType::Plumwine:
		modelPath = "assets/application/Alcohol/Plumwine/Plumwine.obj";
		texturePath = "assets/application/Alcohol/Plumwine/Plumwine.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case GlassType::Whiskey:
        modelPath = "assets/application/Alcohol/Whiskey/Whiskey.obj";
        texturePath = "assets/application/Alcohol/Whiskey/Whiskey.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
        break;
    case GlassType::GLASS_MAX:
    default:
        modelPath = "assets/application/model/Cocktail/Cocktail.obj";
        texturePath = "assets/engine/texture/white1x1.png";
        myColliderTag = CollisionTag::GetTag("Obstacles");
        targetColliderTag = (CollisionTag::GetTag("Glass"));
        mass = 10.0f;
        color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

        break;
    }

    //モデルとテクスチャIDをセットする
    glassObj_->modelID_ = Game::Asset::Model::Load(modelPath);
    textureID_ = Game::Asset::Texture::Load(texturePath);

    comCollider_.CreateFromModelData(
        glassObj_->modelID_,
        worldMatrix_,
		myColliderTag,
		targetColliderTag);

    if (type == GlassType::Glass)
    {
        if (!comCollider_.colliders.empty())
        {
            // 自分のコライダーを変数に保持
            auto& myCollider = comCollider_.colliders.at(0);

            myCollider->SetOnCollisionCallback([this](Collider* collider)
                {

                    bool isCollisionResponse = false;
                    if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Target"))
                    {
                        //ターゲットだったら
                        isCollisionResponse = true;
                    }
                    if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Obstacles"))
                    {
                        //障害物だったら 押し戻す
                        isCollisionResponse = true;

                    }

                    if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Table"))
                    {
                        //テーブルだったら
                    }

                    if (isCollisionResponse)
                    {
                        transform_.translate += comCollider_.colliders.at(0)->GetPhysicsBody().penetration * Game::Time::GetScaledDeltaTimeMs() * 0.001f;
                    }


                });
        }
    }

    comCollider_.colliders.at(0)->SetMass(mass);
    //非弾性衝突　としてみると
    comCollider_.colliders.at(0)->SetCoefficiendOfRestituion(0.0f);
}
