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
    void Draw(const int32_t uiRenderTextureID);
    void DrawImGui();
    float GetTimer() { return gameTimer_; };

	BreakEvaluation* GetBreakEvaluation() { return breakEvaluation_.get(); }

    void SetScore(float score);
    void AddScore(float score);
    int32_t GetScore() const;

    void SetCameraPhasePtr(CameraPhase* cameraPhase);
private:
    CameraPhase* cameraPhasePtr_ = nullptr;
    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

    bool isDown_ = true;
    float aniTimer_ = 0.0f;

    float gameTimer_ = 0.0f;
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
    std::unique_ptr<UIModel>yenWord_ = nullptr;

    //利益
    std::unique_ptr<NumMeshs>benefitNumMesh_ = nullptr;
    //時間
    std::unique_ptr<NumMeshs>timeNumMesh_ = nullptr;


};

