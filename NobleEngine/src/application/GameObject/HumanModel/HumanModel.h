#pragma once
#include<Game.h>
#include"../../System/CompoundCollider/CompoundCollider.h"

class HumanModel
{
public:
    HumanModel();
    ~HumanModel();

    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(int32_t renderTargetID);
    void DrawImGui();
    //コライダーをゲットする
    std::vector<std::unique_ptr<Collider>>& GetColliders() { return comCollider_.colliders; };

	void SetTranslate(Vector3 translate) { transform_.translate = translate; }

    void SetLightData(LightDataForGPU* lightData) { lightData_ = lightData; };

private:
    LightDataForGPU* lightData_;

    std::unique_ptr<RenderObject>obj_ = nullptr;

    EulerTransforms transform_;
    Matrix4x4 worldMatrix_;
    Matrix4x4 wvpMatrix_;
    Vector4 color_;
    Vector3 velocity_;
    Vector3 cameraPos_;
    Material material_;
    int32_t modelID_ = -1;
    int32_t textureID_ = -1;

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


    Collision::CompoundCollider comCollider_;
};

