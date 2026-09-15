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
	comCollider_.CreateFromModelData(modelID_, worldMatrix_,
        /*CollisionTag::GetTag("Target") |ターゲットとしない*/
        CollisionTag::GetTag("Customer"),
        CollisionTag::GetTag("Glass")
    );

	currentAnimationName_ = "IdleSit";

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

void Customer::UpdateAnimation()
{

    //アニメーションが終了したら
    if (isEndAnimation_) {
        currentAnimationName_ = "IdleSit";
    }

    //前フレームで予測線が当たっており、誰かがショットしてた時等ショット
    if (isHitPrediction_) {
        currentAnimationName_ = "Catch";
 
    }

    isHitPrediction_ = false;

}
