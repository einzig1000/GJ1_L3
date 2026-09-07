#pragma once
#include <Game.h>
#include <memory>
#include <vector>
#include <System/CompoundCollider/CompoundCollider.h>

class Collider;
class GlassParticle;

class Obstacle
{
public:
    Obstacle();
    ~Obstacle();
    /// @brief 床との当たり判定を一旦y座標によって判定する
    /// @return 床との当たり判定
    bool GetIsHitFloor() { return isHitFloor_; };
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
    /// @brief グラスタイプを持たせるか基底クラスにするかは考える
    /// @param type 
    void SetGlassTypeAndLoadModels(const GlassType type);
    GlassType GetGlassType() const { return glassType_; }

    void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
	Vector3 GetTranslate() const { return transform_.translate; };

	void SetColor(const Vector4& color) { color_ = color; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };
private:
    //床との当たり判定
    bool isHitFloor_ = false;

	GlassType glassType_ = GlassType::GLASS_MAX;

    //グラス
    std::unique_ptr<RenderObject> glassObj_ = nullptr;

    //テクスチャID
    int32_t textureID_ = -1;

    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Vector4 color_ = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
    //複合コライダー
    Collision::CompoundCollider comCollider_;

    //パーティクル
    std::unique_ptr<GlassParticle>glassParticle_ = nullptr;
};

