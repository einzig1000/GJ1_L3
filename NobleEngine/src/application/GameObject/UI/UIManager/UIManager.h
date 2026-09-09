#pragma once
#include"Game.h"
#include<unordered_map >
class UIModel;
class NumMeshs;
class BreakEvaluation;

    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

class UIManager
{


public:
    UIManager();
    ~UIManager();
    void Initialize();
    void Update();
    void Draw();
    void DebugImGui();
    float GetTimer() { return gameTimer_; };

	BreakEvaluation* GetBreakEvaluation() { return breakEvaluation_.get(); }
    int32_t GetRenderTextureID() const { return renderTextureID_; }

    void SetScore(float score);
    void AddScore(float score);
    int32_t GetScore() const;

private:

    bool isInitializeCamera_ = false;
    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

    float gameTimer_ = 0.0f;

    int32_t renderTextureID_ = -1;
    //UIのcamera
    int32_t uiCameraID_ = -1;

    std::unique_ptr<BreakEvaluation>breakEvaluation_ = nullptr;

    std::unordered_map<std::string, modelIDs>modelIds_;

    //時間とお金
    std::unique_ptr<UIModel>timeAndMoneySignboard_ = nullptr;

    //カクテル
    std::unique_ptr<UIModel>cockTailSignboard_ = nullptr;
    //時間文字
    std::unique_ptr<UIModel>timeWord_ = nullptr;

    //円文字
    std::unique_ptr<UIModel>yenWorl_ = nullptr;

    //利益
    std::unique_ptr<NumMeshs>benefitMesh_ = nullptr;
    //時間
    std::unique_ptr<NumMeshs>timeMesh_ = nullptr;


};

