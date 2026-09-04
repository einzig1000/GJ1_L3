#pragma once

#include <list>
#include"../CollisionInfo.h"
#include "../../../engine/EngineDefinition/EngineDefinition.h"

class Collider;
class Camera;

namespace Collision {
    CollisionInfo GetCollisionInfo(const Sphere& sphere, const AABB& AABB);
    CollisionInfo GetCollisionInfo(const AABB& a, const AABB& b);
    void ResolveCollision(Vector3& pos, Vector3& velocity, const CollisionInfo& info);
    Sphere GetSphereWorldPos(Collider* sphere);
    AABB GetAABBWorldPos(Collider* aabb);

    struct Circle {
        Vector2 center;
        float radius;
    };
    float Distance(const Circle& p1, const Circle& p2) {
        Vector2 distance = p1.center - p2.center;
        distance.Length();
        return  distance.Length();
    }

    bool IsCollision(const Circle& c1, const Circle& c2)
    {
        //2つの急の中心点間距離を求める 
        if (Distance(c1, c2) <= c1.radius + c2.radius) {
            return true;
        }

        return false;
    }

}

/// @brief 衝突マネージャ
class CollisionManager {
public:

    void Load();

    /// @brief コライダーを追加する
    /// @param collider コライダー
    void AddCollider(Collider* collider) {
        colliders_.emplace_back(collider);
    }


    /// @brief コライダーをクリアする
    void ClearColliders() { colliders_.clear(); }
    /// @brief 衝突判定と応答
    void CheckAllCollisions();


    //終了処理
    void Finalize();

    //ImGuiにおける走査
    void DebugImGui();
    //コライダー描画のための更新
    void DebugUpdate(const int32_t cameraID);
    //コライダーデバック描画
    void DebugDraw();

private:


    // コライダーのリスト
    std::list<Collider*> colliders_;

    void  CheckCollisionPair(Collider* a, Collider* b);
    void OnCollision(Collider* a, Collider* b);

    /// @brief XZサークル同士の衝突判定
    /// @param colliderA コライダーA
    /// @param colliderB コライダーB
    void CheckCollisionCirclePair(Collider* colliderA, Collider* colliderB);

    /// @brief コライダー2つの衝突判定と応答
    /// @param colliderA コライダーA
    /// @param colliderB コライダーB
    void CheckCollisionSpherePair(Collider* colliderA, Collider* colliderB);

/// @brief コライダー2つの衝突判定と応答
/// @param colliderA コライダーA
/// @param colliderB コライダーB
    void CheckCollisionAABBPair(Collider* colliderA, Collider* colliderB);
 /// @brief コライダー2つの衝突判定と応答
/// @param colliderA コライダーA
/// @param colliderB コライダーB
    void CheckCollisionSphereAABBPair(Collider* colliderA, Collider* colliderB);
};