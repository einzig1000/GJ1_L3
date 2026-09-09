#pragma once
#include <Game.h>
#include <memory>
#include <vector>
#include <System/CompoundCollider/CompoundCollider.h>

class Collider;

enum class GimmickType
{
	None,
	Spring,
};
class GlassParticle;

class TableObject
{
public:
    TableObject();
    ~TableObject();
    /// @brief 床との当たり判定を一旦y座標によって判定する
    /// @return 床との当たり判定
    bool GetIsHitFloor() { return isHitFloor_; };
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(int32_t renderTargetID);
    void DrawImGui();
    /// @brief グラスタイプを持たせるか基底クラスにするかは考える
    /// @param type 
    void SetGlassTypeAndLoadModels(const GlassType type);
    GlassType GetGlassType() const { return glassType_; }

    void SetTranslate(const Vector3& translate) { transform_.translate = translate; };
	Vector3 GetTranslate() const { return transform_.translate; };
	EulerTransforms GetTransform() const { return transform_; };
    void SetVelocity(const Vector3& vel) { comCollider_.colliders.at(0)->SetVelocity(vel); };

	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; };

	void SetColor(const Vector4& color) { color_ = color; };

    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };

    void SetLightData(LightDataForGPU* lightData) { lightData_ = lightData; };

private:
    LightDataForGPU* lightData_;
    
    //床との当たり判定
    bool isHitFloor_ = false;

	GlassType glassType_ = GlassType::GLASS_MAX;

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

    //複合コライダー
    Collision::CompoundCollider comCollider_;

    //パーティクル
    std::unique_ptr<GlassParticle>glassParticle_ = nullptr;
};

