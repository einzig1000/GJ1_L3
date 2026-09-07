#pragma once
#include<Game.h>

class Numbers
{
private:
    // モデルID
    static std::vector<int32_t> modelIDs_;
    //グラス
    std::unique_ptr<RenderObject> obj_ = nullptr;

    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Vector4 color_;
 
public:
    static void Load();
    Numbers();
    ~Numbers();
    void SetStencil(const DepthStencilID id);
    void Initialize(const uint32_t number, const Vector3& position, const Vector3& rotation = {0.0f,0.0f,0.0f},const Vector3& scale = {1.0f,1.0f,1.0f});
    void Update(const int32_t cameraID);
    void SetModelId(const uint32_t number);
    void Draw();
    void DrawImGui();
};

