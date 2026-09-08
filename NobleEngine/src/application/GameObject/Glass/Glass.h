#pragma once
#include<Game.h>
#include<memory>
#include<vector>
#include"../../System/CompoundCollider/CompoundCollider.h"
class Collider;
class GlassParticle;

class Glass
{
public:
    enum GlassType
    {
        GLASS_COCKTAIL,
        GLASS_MAX,
    };
public:
    Glass();
    ~Glass();
    /// @brief 床との当たり判定を一旦y座標によって判定する
    /// @return 床との当たり判定
    bool GetIsHitFloor() { return isHitFloor_; };
    /// @brief 壊れたフラグの取得
    /// @return 
    bool GetIsBroken() { return isBroken_; }

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
    /// @brief グラスタイプを持たせるか基底クラスにするかは考える
    /// @param type 
    void SetGlassTypeAndLoadModels(const GlassType type);

	void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
	void AddTranslate(const Vector3& translate) { transform_.translate += translate; };

	Vector3 GetTranslate() { return transform_.translate; };
	//void SetVelocity(const Vector3& vel) { velocity_ = vel; };
    Vector3 GetVelocity() { return velocity_; };
    void SetVelocity(const Vector3& vel) { comCollider_.colliders.at(0)->SetVelocity(vel); };

    float GetRadius() { return transform_.scale.x * 1.0f; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };
private:
    //床との当たり判定
    bool isHitFloor_ = false;
    //壊れたフラグ
    bool isBroken_ = false;
    //グラス
    std::unique_ptr<RenderObject> glassObj_ = nullptr;

    // モデルID
    int32_t modelID_ = -1;
    //テクスチャID
    int32_t textureID_ = -1;

    //インスタンス数に応じてそれぞれの構造を持たせる
    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Vector4 color_;
	Vector3 velocity_;

    Collision::CompoundCollider comCollider_;
    //パーティクル
    std::unique_ptr<GlassParticle>glassParticle_ = nullptr;
};

