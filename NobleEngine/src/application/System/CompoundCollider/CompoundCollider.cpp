#include "CompoundCollider.h"
#include"../Collider/Collider.h"
#include"Utilities/Logger/Logger.h"

namespace Collision {

    void SettingColliderFromModelData(
        Collider* collider,
        const int32_t modelID,
        Matrix4x4& mat,
        const uint32_t attribute,
        const uint32_t mask,
        const Collider::ColliderType colliderType) {

        const ModelData* modelData = Game::Asset::Model::GetData(modelID);

        size_t colliderCount = 0;
        if (colliderType == Collider::ColliderType::kColliderType_XZ_Circle) {
            colliderCount = modelData->colliderShape.spheres.size();
        } else if (colliderType == Collider::ColliderType::kColliderType_Sphere) {
            //本ゲームにおいてはここは基本使用しないが
            colliderCount = modelData->colliderShape.spheres.size();
        } else if (colliderType == Collider::ColliderType::kColliderType_AABB) {
            //本ゲームにおいてはここは基本使用しないが
            colliderCount = modelData->colliderShape.aabbs.size();
        }

        if (colliderCount == 0) {
            Log("No Collider!!!");
            return;
        } else if (colliderCount > 1) {
            //1以上 複合コライダーを使った方がいいですよ！
            Log("To Much Collider!!! Use CompoundCollider");
            return;
        }

        //コライダーがあればここに入れる
        collider->SetWorldMatrixAddress(mat);
        collider->SetCollisionAttribute(attribute);
        collider->SetCollisionMask(mask);


        if (colliderType == Collider::ColliderType::kColliderType_XZ_Circle) {
            // Circleのセット
            collider->SetSphere(modelData->colliderShape.spheres[0],true);
        } else if (colliderType == Collider::ColliderType::kColliderType_Sphere) {
            //本ゲームにおいてはここは基本使用しないが サークルではない
            collider->SetSphere(modelData->colliderShape.spheres[0], false);
        } else if (colliderType == Collider::ColliderType::kColliderType_AABB) {
            //本ゲームにおいてはここは基本使用しないが
            collider->SetAABB(modelData->colliderShape.aabbs[0]);
        }
    }


    void CompoundCollider::CreateFromModelData(
        const int32_t modelID,
        Matrix4x4& mat,
        const uint32_t attribute,
        const uint32_t mask)
    {
        colliders.clear();
        const ModelData* modelData = Game::Asset::Model::GetData(modelID);

        size_t aabbCount = modelData->colliderShape.aabbs.size();
        size_t sphereCount = modelData->colliderShape.spheres.size();
        size_t maxCount = aabbCount + sphereCount;

        std::vector<std::unique_ptr<Collider>> newColliders;
        newColliders.reserve(maxCount);

        for (int j = 0; j < maxCount; ++j) {
            auto collider = std::make_unique<Collider>();
            collider->SetWorldMatrixAddress(mat);
            collider->SetCollisionAttribute(attribute);
            collider->SetCollisionMask(mask);
            newColliders.push_back(std::move(collider));
        }
        // 2. Sphere のセット
        for (int j = 0; j < sphereCount; ++j) {
            newColliders[j]->SetSphere(modelData->colliderShape.spheres[j]);
        }

        for (int j = 0; j < aabbCount; ++j) {
            newColliders[sphereCount + j]->SetAABB(modelData->colliderShape.aabbs[j]);
        }

        colliders = std::move(newColliders);

    }
}