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
    void SetVelocity(const Vector3& normal) { emitter_.velocity = normal; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return colliders_; };

    struct PredictionEmitter {
        EulerTransforms transform{};
        Vector3 velocity;
        float kSpeed = 5.0f;
        //生存時間
        float lifeTime = 1.0f;
        float frequencyTime = 0.0f;
        float frequency = 0.125f;
    };

    struct PredictionParam {
        float lifeTime  = 2.0f;
        bool isAlive = false;
    };
private:
    //描画用
    void InitializeForDrawPrediction();
    void UpdateForDrawPrediction(int32_t cameraID);

private:
    //仮に球体のオブジェクトとする

    PredictionEmitter emitter_{};

    // モデルID
    int32_t modelID_ = -1;
    //テクスチャID
    int32_t textureID_ = -1;
    //インスタンス数
    int32_t instanceCount_ = 24;
    
    //インスタンス数に応じてそれぞれの構造を持たせる
    std::vector<EulerTransforms> transforms_;
    std::vector<Matrix4x4> worldMatrices_;

    //インスタンス数に応じて
    std::vector <std::unique_ptr<Collider>> colliders_;
    //生存時間
    std::vector <PredictionParam >param_;

// ================描画用====================

    //仮に球体のオブジェクトとする
    std::unique_ptr<RenderObject> drawObj_ = nullptr;

    // ディスクリプタヒープスロット
    int32_t worldMatrixHeapSlotForDraw_ = -1;
    int32_t colorHeapSlotForDraw_ = -1;
    int32_t textureIndexHeapSlotForDraw_ = -1;

    std::vector<Vector4> colorsForDraw_;
    std::vector<int32_t> textureIndicesForDraw_;

};

