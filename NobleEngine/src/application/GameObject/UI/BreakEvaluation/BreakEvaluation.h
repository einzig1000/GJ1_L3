#pragma once
#include"Game.h"
#include<unordered_map >
class UIModel;
class NumMeshs;

class BreakEvaluation
{
public:
    BreakEvaluation();
    ~BreakEvaluation();
    void Initialize();
    void Update(const int32_t uiCameraId);
    void Draw(const int32_t renderTextureID);
    void DebugImGui();
    void SetBreakCount(const int32_t breakCount);
    void SetMaxBreakCount(const int32_t maxBreakCount);
private:
    void BreakJudgement();

private:
    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };
    std::unordered_map<std::string, modelIDs>modelIds_;

    int32_t maxBreakCount_ = 0;
    int32_t breakCount_ = 0;
    std::string evaluationString_ = "unKnown";

    //破壊数
    std::unique_ptr<UIModel>breakSignboard_ = nullptr;
    //破壊数
    std::unique_ptr<NumMeshs>breakCountMesh_ = nullptr;
    //破壊文字
    std::unique_ptr<UIModel>breakWorl_ = nullptr;

    //判定たち
    std::unordered_map<std::string, std::unique_ptr<UIModel>> evaluations_;
};

