#pragma once

#include"Game.h"
class Numbers;

class NumMeshs
{
public:
    NumMeshs();
    ~NumMeshs();
    /// @brief 初期化
    /// @param maxDigit　最大桁 
    /// @param startPos 初期位置
    void Initialize(const uint32_t maxDigit,const Vector3& startPos,const Vector3& rotate,const Vector3& scale);
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
    //利益の設定 
    void SetValue(const int32_t benefit) {
    value_ = benefit;
    isUpdateValue_ = true;
     };

private:
    bool isMinus_ = false;
    int32_t value_ = 0;
    bool isUpdateValue_ = false;
    //6桁
    uint32_t maxDigit = 6;
    std::vector<std::unique_ptr<Numbers>>numbers_;
    std::unique_ptr<Numbers> minus_ = nullptr;
};

