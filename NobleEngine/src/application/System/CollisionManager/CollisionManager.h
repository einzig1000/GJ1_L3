#pragma once

#include <list>
#include<utility>
#include"../CollisionInfo.h"
#include "../../../engine/EngineDefinition/EngineDefinition.h"

class Collider;
class Camera;

namespace Collision {
    
    struct Circle {
        Vector2 center;
        float radius;
    };
    
    //CollisionInfo GetCollisionInfo(const Sphere& sphere, const AABB& AABB);
    //CollisionInfo GetCollisionInfo(const AABB& a, const AABB& b);
    void ResolveCollision(Vector3& pos, Vector3& velocity, const CollisionInfo& info);
    Sphere GetSphereWorldPos(Collider* sphere);
    Circle GetXZCircleWorldPos(Collider* circle);
    AABB GetAABBWorldPos(Collider* aabb);

    float Distance(const Circle& p1, const Circle& p2);

    bool IsCollision(const Circle& c1, const Circle& c2);
    /// @brief 射影関数
    /// @param v1 ベクトル1
    /// @param v2 ベクトル2
    /// @return 正射影ベクトル
    Vector3 Project(const Vector3& v1, const Vector3& v2);

    /// @brief 物体同士の衝突後の速度を求める
    /// @param pb1 質量と速度1
    /// @param pb2 質量と速度2
    /// @param coefficiendOfRestituion 
    /// @param normal 法線
    /// @return　Vector3のペア 
    std::pair<Vector3, Vector3> ComputeCollisionVelocities(
        const PhysicsBody& pb1,
        const PhysicsBody& pb2,
        float coefficiendOfRestituion,
        const Vector3& normal
    );

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