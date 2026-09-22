#pragma once
#include"Game.h"
#include<unordered_map>

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
    //シェイク
    void SetShakeProgress(const float shake) { shakeProgress_ = shake; };
    void SetCameraPhasePtr(CameraPhase* cameraPhase);
    void SetIsHitCustomer(const bool flag) { isHitCustomer_ = flag; };
private:
    CameraPhase* cameraPhasePtr_ = nullptr;

    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

    // ダウン
    bool isDown_ = true;
    // 顧客にヒットした
    bool isHitCustomer_ = false;
    // アニメタイマー
    float aniTimer_ = 0.0f;
    // ゲームタイマー
    float gameTimer_ = 0.0f;
    // シェイク値
    float shakeProgress_ = 0.0f;

    //UIのcamera
    int32_t uiCameraID_ = -1;

    //モデル
    std::unordered_map<std::string, modelIDs>modelIds_;
   
    //破壊判定
    std::unique_ptr<BreakEvaluation>breakEvaluation_ = nullptr;

    // ==================//UI//=========================
   
    //時間とお金
    std::unique_ptr<UIModel>timeAndMoneySignboard_ = nullptr;
    //カクテル
    std::unique_ptr<UIModel>cockTailSignboard_ = nullptr;
    //時間文字
    std::unique_ptr<UIModel>timeWord_ = nullptr;
    //円文字
    std::unique_ptr<UIModel>yenWord_ = nullptr;
    //カクテル　グラス
    std::unique_ptr<UIModel>glassModel_ = nullptr;

    // ==================//NumMeshs//====================

    //利益
    std::unique_ptr<NumMeshs>benefitNumMesh_ = nullptr;
    //時間
    std::unique_ptr<NumMeshs>timeNumMesh_ = nullptr;
};

