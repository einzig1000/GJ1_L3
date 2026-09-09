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
    //カスタマーとの判定を得る
    bool GetIsHitCustomer() { return isHitCustomer_; }

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(int32_t renderTargetID);
    void DrawImGui();
    /// @brief グラスタイプを持たせるか基底クラスにするかは考える
    /// @param type 
    void SetGlassTypeAndLoadModels(const GlassType type);

	void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
	void AddTranslate(const Vector3& translate) { transform_.translate += translate; };

	Vector3 GetTranslate() { return transform_.translate; };
    Vector3& GetTranslatePointer() { return transform_.translate; };

	//void SetVelocity(const Vector3& vel) { velocity_ = vel; };
    Vector3 GetVelocity() { return velocity_; };
    void SetVelocity(const Vector3& vel) { comCollider_.colliders.at(0)->SetVelocity(vel); };

    float GetRadius() { return transform_.scale.x * 1.0f; };

    //コライダーをゲットする

    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };

	void SetLightData(LightDataForGPU* lightData) { lightData_ = lightData; };

private:
	LightDataForGPU* lightData_;
    //
    bool isHitCustomer_ = false;
    //床との当たり判定
    bool isHitFloor_ = false;
    //壊れたフラグ
    bool isBroken_ = false;
    //グラス
    std::unique_ptr<RenderObject> glassObj_ = nullptr;

    //インスタンス数に応じてそれぞれの構造を持たせる
    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Matrix4x4 wvpMatrix_;
    Vector4 color_;
	Vector3 velocity_;
	Vector3 cameraPos_;
	Material material_;
    int32_t modelID_ = -1;
    int32_t textureID_ = -1;

    Collision::CompoundCollider comCollider_;
    //パーティクル
    std::unique_ptr<GlassParticle>glassParticle_ = nullptr;
};

