#pragma once
#include"Game.h"
#include"../../System/CompoundCollider/CompoundCollider.h"

class PredictionObj
{
public:
    PredictionObj();
    ~PredictionObj();

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();

    void SetTranslate(const Vector3& translate) { emitter_.transform.translate = translate; };
    void SetVelocity(const Vector3& vel) { emitter_.velocity; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return colliders_; };

    struct PredictionEmitter {
        EulerTransforms transform{};
        Vector3 velocity{};
        //生存時間
        float lifeTime = 10.0f;
        float frequencyTime = 0.0f;
        float frequency = 1.0f;
    };

    struct PredictionParam {
        float lifeTime  = 10.0f;
        bool isAlive = false;
    };

private:

    //仮に球体のオブジェクトとする
    std::unique_ptr<RenderObject> obj_ = nullptr;

    PredictionEmitter emitter_{};

    // モデルID
    int32_t modelID_ = -1;
    //テクスチャID
    int32_t textureID_ = -1;

    int32_t instanceCount_ = 12;
    // ディスクリプタヒープスロット
    int32_t worldMatrixHeapSlot_ = -1;
    int32_t colorHeapSlot_ = -1;
    int32_t textureIndexHeapSlot_ = -1;

    //インスタンス数に応じてそれぞれの構造を持たせる
    std::vector<EulerTransforms> transforms_;
    std::vector<Matrix4x4> worldMatrices_;
    std::vector<Vector4> colors_;
    std::vector<int32_t> textureIndices_;

    //インスタンス数に応じて
    std::vector <std::unique_ptr<Collider>> colliders_;
    //生存時間
    std::vector <PredictionParam >param_;
};

