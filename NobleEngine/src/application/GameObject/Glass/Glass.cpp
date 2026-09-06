#include "Glass.h"
#include"Utilities/Json/JsonManager.h"
namespace
{
    //グラス共通の変数
    float deadLine_ = 0.0f;
}

Glass::Glass()
{
    //カクテルをロードする
    SetGlassTypeAndLoadModels(GLASS_COCKTAIL);

    JsonManager::Load("assets/application/json/Glass/Glass.json", "/deadLine", deadLine_);

    transform_.translate.y = 1.28f;



}

Glass::~Glass()
{}

void Glass::Initialize()
{
    //床との当たり判定
    isHitFloor_ = false;

    //レンダーオブジェクトのインスタンス作成
    glassObj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    glassObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    glassObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    glassObj_->SetupFromShaders();

    glassObj_->modelID_ = modelID_;

    //一旦半透明にしておく
    color_ = Vector4{ 1.0f, 1.0f, 1.0f, 0.5f };

    comCollider_.CreateFromModelData(
        modelID_,
        worldMatrix_,
        CollisionTag::GetTag("Glass"),

        CollisionTag::GetTag("Target")|
        CollisionTag::GetTag("Obstacles")
    );

  
      //グラスパーティクル
    glassParticle_ = std::make_unique<GlassParticle>();
  
    if (!comCollider_.colliders.empty()) {

        // 自分のコライダーを変数に保持
        auto& myCollider = comCollider_.colliders.at(0);

        myCollider->SetOnCollisionCallback([this](Collider* collider) {
            
            bool isCollisionResponse = false;
            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Target")) {
               //ターゲットだったら
                isCollisionResponse = true;
            }
            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Obstacles")) {
                //障害物だったら 押し戻す
                isCollisionResponse = true;
              
            }

            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Table")) {
                //テーブルだったら
            }
            
            if (isCollisionResponse) {
                transform_.translate += comCollider_.colliders.at(0)->GetPhysicsBody().penetration*Game::Time::GetScaledDeltaTimeMs()*0.001f;
            }


        });
    }

}

void Glass::Update(const int32_t cameraID)
{
    //毎フレーム当たり判定を初期化する
    isHitFloor_ = false;
    Vector3 vel = { 0.0f };

    //物理を呼ぶぞ！
    if (!comCollider_.colliders.empty())
    {
        //comCollider_.colliders.at(0)->SetVelocity(velocity_);
        //velocity_ *= 0.92f;

        auto  phyB = comCollider_.colliders.at(0)->GetPhysicsBody();
        float mass = phyB.mass;
        vel = phyB.velocity;
    }

    if (transform_.translate.y <= deadLine_) {
        //一旦インスタンス1つで実行　床に衝突、つまり壊れる。
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

void Glass::Draw()
{
    glassObj_->Draw();
}

void Glass::DrawImGui()
{
    ImGui::Begin("GameObj");

    if (ImGui::TreeNode("Glass"))
    {
        static Vector3 vel;
        ImGui::DragFloat3("velocity", &vel.x, 0.1f, -10.0f, 10.0f);
        //物理ボディ
        if (ImGui::TreeNode("PhysicsBody")) {
            if (!comCollider_.colliders.empty()) {
                auto& collider = comCollider_.colliders.at(0);
                auto  phyB = collider->GetPhysicsBody();
                float mass = phyB.mass;

                ImGui::SliderFloat("mass", &phyB.mass, 0.001f, 1000.0f);

                collider->SetMass(phyB.mass);

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
            ImGui::ColorEdit4("Color##", &color_.x);

            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    ImGui::End();
}

void Glass::SetGlassTypeAndLoadModels(const GlassType type)
{

    std::string filePath;
    std::string textureFilePath = "assets/engine/texture/white1x1.png";

    switch (type)
    {
    case Glass::GLASS_COCKTAIL:
        filePath = "assets/application/model/Cocktail/Cocktail.obj";
        break;
    default:
        //デフォルトはカクテル
        filePath = "assets/application/model/Cocktail/Cocktail.obj";
        break;
    }

    //モデルとテクスチャIDをセットする
    modelID_ = Game::Asset::Model::Load(filePath);
    textureID_ = Game::Asset::Texture::Load(textureFilePath);
}
