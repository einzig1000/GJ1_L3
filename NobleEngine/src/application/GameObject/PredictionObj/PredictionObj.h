#pragma once
#include"Game.h"
#include"../../System/CompoundCollider/CompoundCollider.h"

class CollisionManager;
class TableObject;

class PredictionObj
{
public:
    PredictionObj();
    ~PredictionObj();

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();

    void SetTranslate(const Vector3& translate) { emitter_.translate = translate; };
    void SetVelocity(const Vector3& normal) { emitter_.velocity = normal; };

	void SetCollisionManager(CollisionManager* collisionManager) { collisionManager_ = collisionManager; };
	void SetObstacleCount(int32_t count) { obstacleCount_ = count; };
    void SetObstacleArray(std::unique_ptr<TableObject>* obstacles) { obstacles_ = obstacles; };

    //コライダーをゲットする
    //std::vector<std::unique_ptr<Collider>>& GetColliders() { return colliders_; };
    std::unique_ptr<Collider>& GetColliders() { return colliders_; };

    struct PredictionEmitter {
        Vector3 translate;
        Vector3 velocity;
    };

private:
    void CheckColliders();

    // 予測用の仮想衝突が実際の障害物コライダーの物理状態(速度・めり込み量)を
    // 書き換えてしまうため、シミュレーション前後で退避・復元する
    void SnapshotObstaclePhysics();
    void RestoreObstaclePhysics();

    //描画用
    void InitializeForDrawPrediction();
    void UpdateForDrawPrediction(int32_t cameraID);

private:
	CollisionManager* collisionManager_ = nullptr;
	int32_t obstacleCount_ = 0;
    std::unique_ptr<TableObject>* obstacles_ = nullptr;

    //仮に球体のオブジェクトとする

    PredictionEmitter emitter_{};

    // モデルID
    int32_t modelID_ = -1;
    //テクスチャID
    int32_t textureID_ = -1;
    //インスタンス数
    int32_t instanceCount_ = 6;
    
    //インスタンス数に応じてそれぞれの構造を持たせる

    // 実際に衝突判定を行うオブジェクト
    std::unique_ptr<Collider> colliders_;
    EulerTransforms transforms_;
	Matrix4x4 worldMatrix_;


// ================描画用====================

    //仮に球体のオブジェクトとする
    std::unique_ptr<RenderObject> drawObj_ = nullptr;

    // ディスクリプタヒープスロット
    int32_t worldMatrixHeapSlotForDraw_ = -1;
    int32_t colorHeapSlotForDraw_ = -1;
    int32_t textureIndexHeapSlotForDraw_ = -1;


    std::vector<Vector4> colorsForDraw_;
    std::vector<int32_t> textureIndicesForDraw_;
    std::vector<Matrix4x4> worldMatricesForDraw_;



    // 障害物の物理状態(速度・めり込み量)の退避先
    struct ObstaclePhysicsSnapshot
    {
        Collider* collider = nullptr;
        Vector3 velocity;
        Vector3 penetration;
    };
    std::vector<ObstaclePhysicsSnapshot> obstaclePhysicsSnapshot_;
};

