#define NOMINMAX
#include "Collider.h"
#include <Utilities/Logger/Logger.h>


void Collider::InitCalcuatedTisFrameFlag()
{
    isCalculatedThisFrame_ = false;
}

Collider::Collider()
{
    collisionInfo_.collided = false;
    collisionInfo_.normal = { 0.0f,0.0f,0.0f };
    collisionInfo_.penetration = { 0.0f };

    center_ = { 0.0f,0.0f,0.0f };
    float size = 0.5f;
    aabb_ = { {-size ,-size ,-size }, {size,size,size} };

    isCalculatedThisFrame_ = false;
}

Collider::~Collider()
{
}


Vector3 ParentMatrix::GetWorldTransformByMatrix(const Matrix4x4& mat)
{
    return { mat.m[3][0], mat.m[3][1], mat.m[3][2] };
}

const Vector3& Collider::CalculateWorldPos()
{
    if (isCalculatedThisFrame_) {
        return tempWorldTransform_;
    }

    if (worldMat_ == nullptr) {
        Log("ワールド行列がありませんでした。");
        //仮に原点とする
        return { 0.0f,0.0f,0.0f };
    }

    Matrix4x4 child;
    //センターからワールド行列を作成する
    child.MakeTranslateMatrix(center_);
    child = child* *worldMat_;

    //ワールド座標の取得
    tempWorldTransform_ = ParentMatrix::GetWorldTransformByMatrix(child);

    //計算終了
    isCalculatedThisFrame_ = true;

    return tempWorldTransform_;
}

void Collider::OnCollisionCollider()
{
#ifdef _DEVELOP
    // object3d_->SetColor({ 1.0f,0.0f,0.0f,0.5f });
#endif // _DEVELOP

}

