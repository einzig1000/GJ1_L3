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
    void SetNormal(const Vector3& normal) { emitter_.normal = normal; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return colliders_; };

    struct PredictionEmitter {
        EulerTransforms transform{};
        Vector3 normal;
        float kSpeed = 20.0f;
        //生存時間
        float lifeTime = 2.0f;
        float frequencyTime = 0.0f;
        float frequency = 0.25f;
    };

    struct PredictionParam {
        float lifeTime  = 2.0f;
        bool isAlive = false;
    };

    struct DrawPrediction {
        //ヒット座標
        Vector3 hitPos;
        //反射
        Vector3 reflect;
        //ヒットしたか
        bool isHit;
    };

private:
    //描画用
    void InitializeForDrawPrediction();
    void UpdateForDrawPrediction(int32_t cameraID);

private:
    //仮に球体のオブジェクトとする

    PredictionEmitter emitter_{};
    DrawPrediction drawPrediction_{};

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

    float currentTime_ = 0.0f;
    const float hitTime_ = 1.0f;

    //仮に球体のオブジェクトとする

    std::unique_ptr<RenderObject> drawObj_ = nullptr;
    int32_t instanceCountForDraw_ = 11;

    // ディスクリプタヒープスロット
    int32_t worldMatrixHeapSlotForDraw_ = -1;
    int32_t colorHeapSlotForDraw_ = -1;
    int32_t textureIndexHeapSlotForDraw_ = -1;

    //インスタンス数に応じてそれぞれの構造を持たせる
    std::vector<EulerTransforms> transformsForDraw_;
    std::vector<Matrix4x4> worldMatricesForDraw_;
    std::vector<Vector4> colorsForDraw_;
    std::vector<int32_t> textureIndicesForDraw_;

};

