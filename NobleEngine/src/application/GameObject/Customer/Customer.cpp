#include "Customer.h"

Customer::Customer()
{
	Load();
}

Customer::~Customer()
{
}

void Customer::Load()
{
	std::string directory = "assets/application/model/Woman/";
	std::string filePath = directory + "woman.gltf";
	// モデル
	modelID_ = Game::Asset::Model::Load(filePath);
	// アニメーション
    animationIDs_.clear();

    animationIDs_["Catch"] = Game::Asset::Animation::Load(filePath, "Catch");
	animationIDs_["IdleSit"] = Game::Asset::Animation::Load(filePath, "IdleSit");

	// テクスチャ
	textureID_ = Game::Asset::Texture::Load(directory + "texture.png");
	//インスタンス1なので0とし行列のコンテナは考えない
	comCollider_.CreateFromModelData(modelID_, worldMatrix_, CollisionTag::GetTag("Target") | CollisionTag::GetTag("Customer"), CollisionTag::GetTag("Glass"));

	currentAnimationName_ = "IdleSit";

}

void Customer::UpdateAnimation()
{
    transform_.translate.y = 0.5f;

    bool isNear = false;
    if (glassPos_) {
        Vector3 distance = *glassPos_ - transform_.translate;
        distance.y = 0.0f;
        if (distance.Length() < 1.0f) {
            isNear = true;
        };

    }

    if (isNear) {

        if (currentAnimationName_ == "IdleSit") {
            currentAnimationName_ = "Catch";
        } else {
            if (isEndAnimation_) {
                currentAnimationName_ = "IdleSit";
            }

        }


    } else {
        if (isEndAnimation_) {
            currentAnimationName_ = "IdleSit";
        }
    }
}
