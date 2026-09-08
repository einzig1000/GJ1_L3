#include "PredictionObj.h"
#include <System/CollisionManager/CollisionManager.h>
#include <GameObject/TableObject/TableObject.h>
#include <algorithm>



PredictionObj::PredictionObj()
{
    std::string filePath = "assets/engine/model/sphere/sphere.obj";
    std::string textureFilePath = "assets/engine/texture/white1x1.png";
    //モデルとテクスチャIDをセットする
    modelID_ = Game::Asset::Model::Load(filePath);
    textureID_ = Game::Asset::Texture::Load(textureFilePath);

    //レンダーオブジェクトのインスタンス作成
    drawObj_ = std::make_unique<RenderObject>();
    //シンプルモデルのシェーダー適用
    drawObj_->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModels.VS.hlsl";
    drawObj_->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModels.PS.hlsl";
    drawObj_->SetupFromShaders();
    drawObj_->modelID_ = modelID_;
    drawObj_->instanceNum_ = instanceCount_;

    //描画用
    worldMatrixHeapSlotForDraw_ = Game::Resource::CreateDynamic();
    colorHeapSlotForDraw_ = Game::Resource::CreateDynamic();
    textureIndexHeapSlotForDraw_ = Game::Resource::CreateDynamic();
}

PredictionObj::~PredictionObj()
{
}

void PredictionObj::Initialize()
{
    //生存時間 判定用
    worldMatricesForDraw_.resize(instanceCount_, Matrix4x4());
    colorsForDraw_.resize(instanceCount_, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	textureIndicesForDraw_.resize(instanceCount_, textureID_);

    colliders_ = std::make_unique<Collider>();

    //一旦サークルとして扱う
    Collision::SettingCollider(
        colliders_.get(),
        Game::Asset::Model::Load("assets/application/model/Cocktail/Cocktail.obj"),
        worldMatrix_,
        CollisionTag::GetTag("Prediction"),
        CollisionTag::GetTag("Obstacles"),
        Collider::ColliderType::kColliderType_XZ_Circle
    );

    // 自分のコライダーを変数に保持
    auto& myCollider = colliders_;
    myCollider->SetCoefficiendOfRestituion(1.0f);
    auto& myTransform = transforms_;
    myCollider->SetOnCollisionCallback([this, &myCollider, &myTransform](Collider* collider)
        {
            bool isCollisionResponse = false;

            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Obstacles"))
            {
                //障害物だったら 押し戻す
                isCollisionResponse = true;
            }

            if (isCollisionResponse)
            {
                myTransform.translate += myCollider->GetPhysicsBody().penetration * Game::Time::GetScaledDeltaTimeMs() * 0.001f;
            }
        });

    InitializeForDrawPrediction();
}

void PredictionObj::Update(const int32_t cameraID)
{
    //float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
    float deltaTime = 1.0f / 60.0f;

    // 発射フェーズ
    //コライダーの初速度を設定する
    colliders_->SetVelocity(emitter_.velocity);

    //位置をセットする
    transforms_.translate = emitter_.translate;

    // 障害物側の物理状態を退避(仮想衝突での書き換えを後で元に戻すため)
    SnapshotObstaclePhysics();

    // 物理を呼ぶぞ！
    for (int i = 0; i < instanceCount_; ++i)
    {
        Vector3 vel = { 0.0f };
    
        for (int k = 0; k < 10; k++)
        {
            auto phyB = colliders_->GetPhysicsBody();
            vel = phyB.velocity;
			transforms_.translate += vel * deltaTime;
            //vel *= 0.99f;
            //colliders_->SetVelocity(vel);
			worldMatrix_ = transforms_.GetWorldMatrix();
            CheckColliders();
        }
    
		EulerTransforms transformsForDraw = transforms_;
        transformsForDraw.scale = { 0.125f, 0.125f, 0.125f };
        //スケールタイム適用済みのデルタタイムを取得して座標を動かす
        worldMatricesForDraw_[i] = transformsForDraw.GetWorldMatrix();
    }

    //constexpr float kDotSpacing = 0.3f;        // 弾同士の間隔(ワールド単位。要調整)
    //constexpr int32_t kMaxSubStepsPerDot = 60; // 1点あたりの最大サブステップ数(速度0近くでの無限ループ防止)
    //
    //// 物理を呼ぶぞ！
    //for (int i = 0; i < instanceCount_; ++i)
    //{
    //    float travelled = 0.0f;
    //    int32_t subStep = 0;
    //
    //    while (travelled < kDotSpacing && subStep < kMaxSubStepsPerDot)
    //    {
    //        auto phyB = colliders_->GetPhysicsBody();
    //        Vector3 vel = phyB.velocity;
    //
    //        Vector3 delta = vel * deltaTime;
    //        transforms_.translate += delta;
    //        travelled += delta.Length();
    //
    //        vel *= 0.99f;
    //        colliders_->SetVelocity(vel);
    //
    //        worldMatrix_ = transforms_.GetWorldMatrix();
    //        CheckColliders();
    //
    //        ++subStep;
    //    }
    //
    //    EulerTransforms transformsForDraw = transforms_;
    //    transformsForDraw.scale = { 0.125f, 0.125f, 0.125f };
    //    worldMatricesForDraw_[i] = transformsForDraw.GetWorldMatrix();
    //}

    UpdateForDrawPrediction(cameraID);
}

void PredictionObj::Draw()
{
    if (emitter_.velocity.Length() > 0.0f) {
        //速度アリの時は描画する
        drawObj_->Draw();
    }
  
}

void PredictionObj::DrawImGui()
{
    //ImGui::Begin("GameObj");
    //
    //if (!colliders_.empty()) {
    //for (int i = 0; i < instanceCount_; ++i) {
    //    ImGui::PushID(i);
    //    if (ImGui::TreeNode("Predictions"))
    //    {
    //        ImGui::Checkbox("isAlive", &param_[i].isAlive);
    //        ImGui::DragFloat("lifeTime", &param_[i].lifeTime, 0.1f, 0.0f,emitter_.lifeTime);
    //
    //        static Vector3 vel;
    //        ImGui::DragFloat3("velocity", &vel.x, 0.1f, -10.0f, 10.0f);
    //        //物理ボディ
    //        if (ImGui::TreeNode("PhysicsBody")) {
    //   
    //             auto& collider = colliders_[i];
    //             auto  phyB = collider->GetPhysicsBody();
    //             float mass = phyB.mass;
    //
    //             ImGui::SliderFloat3("velocity", &phyB.velocity.x, 0.001f, 1000.0f);
    //             ImGui::SliderFloat("mass", &phyB.mass, 0.001f, 1000.0f);
    //             collider->SetMass(phyB.mass);
    //
    //             if (ImGui::Button("Shot"))
    //             {
    //                 collider->SetVelocity(vel);
    //             }
    //         
    //            ImGui::DragFloat3("Scale", &transforms_[i].scale.x, 0.01f);
    //            ImGui::DragFloat3("Rotate", &transforms_[i].rotate.x, 0.01f);
    //            ImGui::DragFloat3("Translate", &transforms_[i].translate.x, 0.01f);
    //
    //            ImGui::TreePop();
    //        }
    //
    //        ImGui::TreePop();
    //    }
    //
    //    ImGui::PopID();
    //}
    //}
    //ImGui::End();
}

void PredictionObj::CheckColliders()
{
    //コライダーリストを毎フレーム削除してみる？
    collisionManager_->ClearColliders();

    //コライダーを追加する
    for (int32_t i = 0; i < obstacleCount_; i++)
    {
        for (auto& collider : obstacles_[i]->GetColliders())
        {
            collisionManager_->AddCollider(collider.get());
        }
    }

    collisionManager_->AddCollider(colliders_.get());

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();

    // 仮想衝突によって書き換えられた障害物側の速度・めり込み量を元に戻す
    RestoreObstaclePhysics();
}


void PredictionObj::InitializeForDrawPrediction()
{
    //赤
    colorsForDraw_.resize(instanceCount_, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    textureIndicesForDraw_.resize(instanceCount_, textureID_);

}

void PredictionObj::UpdateForDrawPrediction(int32_t cameraID)
{

    Game::Resource::UpdateData(worldMatrixHeapSlotForDraw_, worldMatricesForDraw_);
    Game::Resource::UpdateData(colorHeapSlotForDraw_, colorsForDraw_);
    Game::Resource::UpdateData(textureIndexHeapSlotForDraw_, textureIndicesForDraw_);

    Matrix4x4 viewProjection = Game::Camera::Getter::GetViewProjectionMatrix(cameraID);
    int32_t vsHeapSlot = Game::Resource::GetSRV(worldMatrixHeapSlotForDraw_);
    Vector2uint psHeapSlot{ Game::Resource::GetSRV(colorHeapSlotForDraw_), Game::Resource::GetSRV(textureIndexHeapSlotForDraw_) };

    drawObj_->SetCBufferData(0, ShaderType::VertexShader, &viewProjection);
    drawObj_->SetCBufferData(1, ShaderType::VertexShader, &vsHeapSlot);
    drawObj_->SetCBufferData(0, ShaderType::PixelShader, &psHeapSlot);

}

void PredictionObj::SnapshotObstaclePhysics()
{
    obstaclePhysicsSnapshot_.clear();

    for (int32_t i = 0; i < obstacleCount_; i++)
    {
        for (auto& collider : obstacles_[i]->GetColliders())
        {
            auto phyB = collider->GetPhysicsBody();
            obstaclePhysicsSnapshot_.push_back({ collider.get(), phyB.velocity, phyB.penetration });
        }
    }
}

void PredictionObj::RestoreObstaclePhysics()
{
    for (auto& snapshot : obstaclePhysicsSnapshot_)
    {
        snapshot.collider->SetVelocity(snapshot.velocity);
        snapshot.collider->SetPenetrationVector(snapshot.penetration);
    }
}