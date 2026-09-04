#pragma once
#include<Game.h>

class HumanModel
{
public:
    HumanModel();
    ~HumanModel();

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
private:
    std::unique_ptr<RenderObject>obj_ = nullptr;
    //モデル
    int32_t model_ = -1;
    //テクスチャ
    int32_t texture_ = -1;

    std::string currentAnimationName_ = "Idle";
    //アニメーションID
    std::unordered_map<std::string, int32_t>animationIDs_;

    std::unique_ptr<RenderObject> animation_;
    std::unique_ptr<ComputeObject> animationCompute_;
    float animationTime_ = 0.0f;
    int32_t resultHeapSlot_ = -1;
    uint32_t numVertices_;
    SkinInstance skinInstance_;
    const ModelData* modelData_ = nullptr;

};

