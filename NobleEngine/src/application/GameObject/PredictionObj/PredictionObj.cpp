#include "PredictionObj.h"
#include<algorithm>

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
    drawObj_->instanceNum_ = instanceCountForDraw_;

    //描画用
    worldMatrixHeapSlotForDraw_ = Game::Resource::CreateDynamic();
    colorHeapSlotForDraw_ = Game::Resource::CreateDynamic();
    textureIndexHeapSlotForDraw_ = Game::Resource::CreateDynamic();

    emitter_.transform.scale = { 0.125f,0.125f,0.125f };
}

PredictionObj::~PredictionObj()
{
}

void PredictionObj::Initialize()
{ 
    //生存時間 判定用
    transforms_.resize(instanceCount_, EulerTransforms());
    worldMatrices_.resize(instanceCount_, Matrix4x4());
    colliders_.resize(instanceCount_);
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
            Collider::ColliderType::kColliderType_XZ_Circle,
           4.0f
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
            
                drawPrediction_.isHit = true;
                //描画ヒット座標と反射を記録する
                drawPrediction_.hitPos = myTransform.translate;
                drawPrediction_.reflect = myCollider->GetPhysicsBody().velocity;

            
            }
            
       
            });
    }

    InitializeForDrawPrediction();
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
               //射出と同時にヒットしてないとする
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

   UpdateForDrawPrediction(cameraID);
}

void PredictionObj::Draw()
{
    drawObj_->Draw();
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
  
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
    }
    ImGui::End();
}

void PredictionObj::InitializeForDrawPrediction()
{
    drawPrediction_.hitPos = { 0.0f };
    drawPrediction_.reflect = { 0.0f };

    transformsForDraw_.resize(instanceCountForDraw_, EulerTransforms{ .scale = {0.125f,0.125f,0.125f},.rotate = {0.0f,0.0f,0.0f},.translate = {0.0f,0.0f,0.0f} });
    worldMatricesForDraw_.resize(instanceCountForDraw_, Matrix4x4());
    //赤
    colorsForDraw_.resize(instanceCountForDraw_, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
    textureIndicesForDraw_.resize(instanceCountForDraw_, textureID_);

}

void PredictionObj::UpdateForDrawPrediction(int32_t cameraID)
{

    float deltaTime = Game::Time::GetScaledDeltaTimeMs() * 0.001f;
    currentTime_ -= deltaTime;
    currentTime_ = std::clamp(currentTime_,0.0f,hitTime_);

    if (currentTime_ <= 0.0f) {
        drawPrediction_.isHit = false;
        currentTime_ = hitTime_;
    }

    for (int i = 0; i < instanceCountForDraw_; ++i) {

        if (drawPrediction_.isHit) {
            //描画ヒット座標と反射
            Vector3 allLength = drawPrediction_.hitPos - emitter_.transform.translate;
            Vector3 reflectPos = drawPrediction_.hitPos - drawPrediction_.reflect.Normalized() * allLength;

            if (i < 5) {
                transformsForDraw_[i].translate = Game::Math::Ease::Easing(emitter_.transform.translate, drawPrediction_.hitPos, EaseType::LINEAR, 1.0f / 5.0f * i);
            } /*else if (i == 5) {
                transformsForDraw_[i].translate = Game::Math::Ease::Easing(emitter_.transform.translate, drawPrediction_.hitPos, EaseType::LINEAR, 0.9f);
            } */else {
                transformsForDraw_[i].translate = Game::Math::Ease::Easing(drawPrediction_.hitPos, reflectPos, EaseType::LINEAR, 1.0f / 5.0f * i - 1.0f);
            }
        } else {

           //Vector3 noHitPos = drawPrediction_.hitPos + emitter_.normal *i;
            transformsForDraw_[i].translate = emitter_.transform.translate + emitter_.normal * i *0.5f;
        }

      
        worldMatricesForDraw_[i] = transformsForDraw_[i].GetWorldMatrix();
    }

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
