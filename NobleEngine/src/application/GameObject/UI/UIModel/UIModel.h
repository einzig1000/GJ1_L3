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
    //親行列
    Matrix4x4* parent_ = nullptr;
public:
    UIModel();
    ~UIModel();
    Matrix4x4* GetWorldMatrixPtr() { return &worldMatrix_; };
    void SetStencil(const DepthStencilID id);
    void Initialize(const int32_t modelID,const int32_t textureID, const EulerTransforms& transform,Matrix4x4* parent = nullptr);
    void Update(const int32_t cameraID);
    void Draw(const int32_t renderTexture);
    void DebugUI(const int id);
    void SetRotateX(const float rotate);
};

