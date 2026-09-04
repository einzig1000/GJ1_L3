#pragma once
#include<Game.h>
#include<memory>
#include<vector>
#include"../../System/CompoundCollider/CompoundCollider.h"
class Collider;
class Glass
{
public:
    enum GLASS_TYPE {
        GLASS_COCKTAIL,
        GLASS_MAX,
    };
public:
    Glass();
    ~Glass();
    /// @brief 床との当たり判定を一旦y座標によって判定する
    /// @return 床との当たり判定
    bool GetIsHitFloor() { return isHitFloor_; };
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
    /// @brief グラスタイプを持たせるか基底クラスにするかは考える
    /// @param type 
    void SetGlassTypeAndLoadModels(const GLASS_TYPE type);
    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };
private:
    //床との当たり判定
    bool isHitFloor_ = false;

    //グラス
    std::unique_ptr<RenderObject> glassObj_ = nullptr;

    // モデルID
    int32_t modelID_ = -1;
    //テクスチャID
    int32_t textureID_ = -1;

    //インスタンス数　仮に1としておく
    int32_t instanceCount_ = 1;
    // ディスクリプタヒープスロット
    int32_t worldMatrixHeapSlot_ = -1;
    int32_t colorHeapSlot_ = -1;
    int32_t textureIndexHeapSlot_ = -1;

    //インスタンス数に応じてそれぞれの構造を持たせる
    std::vector<EulerTransforms> transforms_;
    std::vector<Matrix4x4> worldMatrices_;
    std::vector<Vector4> colors_;
    std::vector<int32_t> textureIndices_;

    Collision::CompoundCollider comCollider_;
};

