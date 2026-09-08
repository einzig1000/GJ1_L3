#pragma once
#include<Game.h>

class UIModel
{
private:
    //モデル
    int32_t modelId_ = -1;
    //テクスチャ
    int32_t textureID_ = -1;
    //グラス
    std::unique_ptr<RenderObject> obj_ = nullptr;

    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Vector4 color_;

public:
    UIModel();
    ~UIModel();
    void SetStencil(const DepthStencilID id);
    void Initialize(const int32_t modelID,const int32_t textureID, const Vector3& position, const Vector3& rotation = { 0.0f,0.0f,0.0f }, const Vector3& scale = { 1.0f,1.0f,1.0f });
    void Update(const int32_t cameraID);
    void Draw();
};

