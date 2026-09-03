#include "CompoundCollider.h"
#include"../Collider/Collider.h"

namespace Collision {

    void CompoundCollider::CreateFromModelData(
        const int32_t modelID, 
        Matrix4x4& mat,
        const uint32_t attribute,
        const uint32_t mask)
    {

        ModelData modelData = *Game::Asset::Model::GetData(modelID);

        size_t aabbCount = modelData.colliderShape.aabbs.size();
        size_t sphereCount = modelData.colliderShape.spheres.size();
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

        for (int j = 0; j < aabbCount; ++j) {
            newColliders[j]->SetAABB(modelData.colliderShape.aabbs[j]);
        }

        // 2. Sphere のセット
        for (size_t j = sphereCount; j < sphereCount; ++j) {
            newColliders[j]->SetSphere(modelData.colliderShape.spheres[j]);
        }

        colliders = std::move(newColliders);


    }
}