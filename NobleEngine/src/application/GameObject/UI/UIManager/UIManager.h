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
private:
    struct modelIDs {
        int32_t model_ = -1;
        int32_t texture_ = -1;
    };

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
    std::unique_ptr<UIModel>timeWorld_ = nullptr;
    //破壊文字
    std::unique_ptr<UIModel>breakWorld_ = nullptr;
    //円文字
    std::unique_ptr<UIModel>yenWorld_ = nullptr;
    //利益
    std::unique_ptr<NumMeshs>benefitMesh_ = nullptr;
    //時間
    std::unique_ptr<NumMeshs>timeMesh_ = nullptr;
};

