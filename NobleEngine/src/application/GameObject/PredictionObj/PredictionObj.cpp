#include "PredictionObj.h"

PredictionObj::PredictionObj()
{
    std::string filePath = "assets/engine/model/sphere/sphere.obj";
    std::string textureFilePath = "assets/engine/texture/white1x1.png";
    //モデルとテクスチャIDをセットする
    modelID_ = Game::Asset::Model::Load(filePath);
    textureID_ = Game::Asset::Texture::Load(textureFilePath);

    //レンダーオブジェクトのインスタンス作成
    obj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    obj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
    obj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
    obj_->SetupFromShaders();
    obj_->modelID_ = modelID_;
    obj_->instanceNum_ = instanceCount_;

    worldMatrixHeapSlot_ = Game::Resource::CreateDynamic();
    colorHeapSlot_ = Game::Resource::CreateDynamic();
    textureIndexHeapSlot_ = Game::Resource::CreateDynamic();
}

PredictionObj::~PredictionObj()
{
}

void PredictionObj::Initialize()
{ 
  
    transforms_.resize(instanceCount_, EulerTransforms{ .scale = {10.0f,10.0f,10.0f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} });
    worldMatrices_.resize(instanceCount_, Matrix4x4());
    //緑
    colors_.resize(instanceCount_, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    textureIndices_.resize(instanceCount_, textureID_);

    colliders_.resize(instanceCount_);
    //生存時間
    param_.resize(instanceCount_);

    //最初から出るぞー
    emitter_.frequencyTime = emitter_.frequency;


    for (int i = 0; i < instanceCount_; ++i) {

        param_[i].isAlive = true;
        param_[i].lifeTime = emitter_.lifeTime;

        colliders_[i] = std::make_unique<Collider>();
        //一旦サークルとして扱う
        Collision::SettingCollider(
            colliders_[i].get(), 
            worldMatrices_[i],
            CollisionTag::GetTag("Prediction"),
            CollisionTag::GetTag("Target") |
            CollisionTag::GetTag("Obstacles"),
            Collider::ColliderType::kColliderType_XZ_Circle
        );

        // 自分のコライダーを変数に保持
        auto& myCollider = colliders_[i];
        auto& myTransform = transforms_[i];
        myCollider->SetOnCollisionCallback([this, &myCollider, &myTransform](Collider* collider) {

            bool isCollisionResponse = false;
            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Target")) {
                //ターゲットだったら

            }
            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Obstacles")) {
                //障害物だったら 押し戻す
                isCollisionResponse = true;
            }

            if (isCollisionResponse) {
                myTransform.translate += myCollider->GetPhysicsBody().penetration * Game::Time::GetScaledDeltaTimeMs() * 0.001f;
            }

            });
    }


}

void PredictionObj::Update(const int32_t cameraID)
{

    float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
    emitter_.frequencyTime += deltaTime;
  
    if (emitter_.frequency <= emitter_.frequencyTime)
    {
        emitter_.frequencyTime -= emitter_.frequency;

        for (int i = 0; i < instanceCount_; ++i) {
            if (!param_[i].isAlive) {
                param_[i].isAlive = true;
              //コライダーの初速度を設定する
                colliders_[i]->SetVelocity(emitter_.normal* emitter_.kSpeed);
                //位置をセットする
                transforms_[i] = emitter_.transform;
                //一度設定したらループを抜ける
                break;
            }
        }  
    }

    //物理を呼ぶぞ！
    for (int i = 0; i < instanceCount_;++i) {

        if (param_[i].isAlive) {
            param_[i].lifeTime -= deltaTime;

            if (param_[i].lifeTime <= 0.0f) {
                param_[i].isAlive = false;
                //エミッター共通のライフタイムを入れる
                param_[i].lifeTime = emitter_.lifeTime;
            }
            Vector3 vel = { 0.0f };

            auto  phyB = colliders_[i]->GetPhysicsBody();
            float mass = phyB.mass;
            vel = phyB.velocity;

            //スケールタイム適用済みのデルタタイムを取得して座標を動かす
            transforms_[i].translate += vel * Game::Time::GetScaledDeltaTimeMs() * 0.001f;
        } else {
            //下方向に退避する
            transforms_[i].translate = { 0.0f,-10.0f,0.0f };
            //一応初期化する
            colliders_[i]->SetVelocity({0.0f,0.0f,0.0f});
        }

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

void PredictionObj::Draw()
{
    obj_->Draw();
}

void PredictionObj::DrawImGui()
{
    ImGui::Begin("GameObj");

    if (!colliders_.empty()) {
    for (int i = 0; i < instanceCount_; ++i) {
        ImGui::PushID(i);
        if (ImGui::TreeNode("Predictions"))
        {
            ImGui::Checkbox("isAlive", &param_[i].isAlive);
            ImGui::DragFloat("lifeTime", &param_[i].lifeTime, 0.1f, 0.0f,emitter_.lifeTime);

            static Vector3 vel;
            ImGui::DragFloat3("velocity", &vel.x, 0.1f, -10.0f, 10.0f);
            //物理ボディ
            if (ImGui::TreeNode("PhysicsBody")) {
       
                 auto& collider = colliders_[i];
                 auto  phyB = collider->GetPhysicsBody();
                 float mass = phyB.mass;

                 ImGui::SliderFloat3("velocity", &phyB.velocity.x, 0.001f, 1000.0f);
                 ImGui::SliderFloat("mass", &phyB.mass, 0.001f, 1000.0f);
                 collider->SetMass(phyB.mass);

                 if (ImGui::Button("Shot"))
                 {
                     collider->SetVelocity(vel);
                 }
             
                ImGui::DragFloat3("Scale", &transforms_[i].scale.x, 0.01f);
                ImGui::DragFloat3("Rotate", &transforms_[i].rotate.x, 0.01f);
                ImGui::DragFloat3("Translate", &transforms_[i].translate.x, 0.01f);
                ImGui::ColorEdit4("Color", &colors_[i].x);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
    }
    ImGui::End();
}
