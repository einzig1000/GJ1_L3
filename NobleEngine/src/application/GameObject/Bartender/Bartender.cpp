#include "Bartender.h"
Bartender::Bartender()
{




}

Bartender::~Bartender()
{
}

void Bartender::Load()
{
    std::string directory = "assets/application/model/Bartender/";
    std::string filePath = directory + "bartender.gltf";
    // モデル
    modelID_ = Game::Asset::Model::Load(filePath);
    
    animationIDs_.clear();

    // アニメーション
    animationIDs_["Idle"] = Game::Asset::Animation::Load(filePath, "Idle");
    //発射
    animationIDs_["ShotGrass"] = Game::Asset::Animation::Load(filePath, "ShotGrass");
    //キャッチ
    animationIDs_["CatchGrass"] = Game::Asset::Animation::Load(filePath, "CatchGrass");

    // テクスチャ
    textureID_ = Game::Asset::Texture::Load(directory + "texture_body.png");
    //インスタンス1なので0とし行列のコンテナは考えない
    comCollider_.CreateFromModelData(modelID_, worldMatrix_, CollisionTag::GetTag("Target"), CollisionTag::GetTag("Glass") | CollisionTag::GetTag("Table"));
    currentAnimationName_ = "Idle";


    if (!comCollider_.colliders.empty()) {

        // 自分のコライダーを変数に保持
        auto& myCollider = comCollider_.colliders.at(0);

        myCollider->SetOnCollisionCallback([this](Collider* collider) {

            if (collider->GetCollisionAttribute() == CollisionTag::GetTag("Prediction")) {

                //予測線と当たった時を得る
                isHitPrediction_ = true;

            }

         });
    }
}

void Bartender::UpdateAnimation()
{
    //アニメーションが終了したら
    if (isEndAnimation_) {
        currentAnimationName_ = "Idle";
    }

    //前フレームで予測線が当たっており、誰かがショットしてた時等ショット
    if (isHitPrediction_) {
        currentAnimationName_ = "CatchGrass";
     
    }

    isHitPrediction_ = false;

}
