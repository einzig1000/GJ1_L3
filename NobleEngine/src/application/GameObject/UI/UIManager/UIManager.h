#pragma once
#include"Game.h"
#include<unordered_map >
class UIModel;
class NumMeshs;

class UIManager
{
public:
    UIManager();
    ~UIManager();
    void Initialize();
    void Update();
    void Draw();
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

    int32_t maxBreakCount_ = 0;
    int32_t breakCount_ = 0;
    std::string evaluationString_ ="unKnown";

    int32_t renderTextureID_ = -1;
    //UIのcamera
    int32_t uiCameraID_ = -1;
    std::unordered_map<std::string, modelIDs>modelIds_;

    //時間とお金
    std::unique_ptr<UIModel>timeAndMoneySignboard_ = nullptr;
    //破壊数
    std::unique_ptr<UIModel>breakSignboard_ = nullptr;
    //カクテル
    std::unique_ptr<UIModel>cockTailSignboard_ = nullptr;
    //時間文字
    std::unique_ptr<UIModel>timeWord_ = nullptr;
    //破壊文字
    std::unique_ptr<UIModel>breakWorl_ = nullptr;
    //円文字
    std::unique_ptr<UIModel>yenWorl_ = nullptr;
    //判定たち
    std::unordered_map<std::string, std::unique_ptr<UIModel>> evaluations_;
    //利益
    std::unique_ptr<NumMeshs>benefitMesh_ = nullptr;
    //時間
    std::unique_ptr<NumMeshs>timeMesh_ = nullptr;
    //破壊数
    std::unique_ptr<NumMeshs>breakCountMesh_ = nullptr;
};

