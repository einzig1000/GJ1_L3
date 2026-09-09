#pragma once
#include<Game.h>
#include"../../System/CompoundCollider/CompoundCollider.h"

class Table
{
public:
    Table();
    ~Table() = default;
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(int32_t renderTargetID);
    void DrawImGui();
    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };

    float GetRadius() { return transform_.scale.x * 2.8f; };
    Vector3 GetTranslate() { return transform_.translate; };
    EulerTransforms GetEulerTransforms() { return transform_; };

    void SetLightData(LightDataForGPU* lightData) { lightData_ = lightData; };

private:
    LightDataForGPU* lightData_;

    //グラス
    std::unique_ptr<RenderObject> obj_ = nullptr;

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


};

