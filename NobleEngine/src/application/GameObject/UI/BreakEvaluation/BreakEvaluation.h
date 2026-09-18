#pragma once
#include"Game.h"
#include<unordered_map >
class UIModel;
class NumMeshs;

class BreakEvaluation
{
public:

    struct AniTime {
        bool isEnd = false;
        float timer = 0.0f;
    };

    BreakEvaluation();
    ~BreakEvaluation();
    void SetIsGlassOutOfTable(const bool flag) { isGlassOutOfTable_ = flag; };
    void Initialize();
    void Update(const int32_t uiCameraId);
    void Draw(const int32_t renderTextureID);
    void DebugImGui();
    //外部から呼び出す
    void SetBreakCount(const int32_t breakCount);
    int32_t GetBreakCount() { return breakCount_; }
    //外部から呼び出す
    void SetMaxBreakCount(const int32_t maxBreakCount);
    //上に上がる
    void SetIsUp(const bool isUp) { isUp_ = isUp; };
    bool GetIsAddScore() { return isAddScore_; };
private:
    void BreakJudgement();
    //カウントが更新されたらアニメーションを開始する
    EulerTransforms AnimationStart(AniTime& aniTime);
    EulerTransforms Easing(const EulerTransforms& start, const EulerTransforms& end, const EaseType type, float time);
private:
    float boardAnimationTimer_ = 0.0f;
    bool isJudgeStart_ = false;
    bool isGlassOutOfTable_ = false;

    bool isAddScore_ = false;
    bool isUp_ = false;
    bool isPreUp_ = false;
    bool isAnimation_ = false;

    struct NumKeyframe{
        EulerTransforms transform_;
        float time = 0.0f;
    };
    std::unordered_map <std::string, NumKeyframe>transformKey_;

    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };
    std::unordered_map<std::string, modelIDs>modelIds_;

    int32_t maxBreakCount_ = 0;
    int32_t breakCount_ = 0;
    std::string evaluationString_ = "unKnown";

    AniTime numberAniTime_;
    AniTime evalutionAniTime_;

    //破壊数
    std::unique_ptr<UIModel>breakSignboard_ = nullptr;
    //現在の破壊数を記録する
    std::unique_ptr<NumMeshs>currentCountMesh_ = nullptr;
    //破壊文字
    std::unique_ptr<UIModel>breakWord_ = nullptr;
    //判定たち
    std::unordered_map<std::string, std::unique_ptr<UIModel>> evaluations_;
};

