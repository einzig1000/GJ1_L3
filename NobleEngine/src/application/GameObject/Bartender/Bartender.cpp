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
}

void Bartender::UpdateAnimation()
{
    bool isNear = false;
    if (glassPos_) {
        Vector3 distance = *glassPos_ - transform_.translate;
        distance.y = 0.0f;
        if (distance.Length() < 1.0f) {
            isNear = true;
        };

    }

    if (isShot_) {
        if (currentAnimationName_ != "ShotGrass") {
            currentAnimationName_ = "ShotGrass";
        }
    } else {
        if (isNear) {
            if (currentAnimationName_ == "Idle"|| currentAnimationName_ == "ShotGrass"&& isEndAnimation_) {
                currentAnimationName_ = "CatchGrass";
            }
          
        } else {
            if (isEndAnimation_) {
                currentAnimationName_ = "Idle";
            }
      
        }
    }

}
