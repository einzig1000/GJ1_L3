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
    void Initialize(const uint32_t maxDigit,const EulerTransforms& transform,Matrix4x4* parent = nullptr);
    void Update(const int32_t cameraID);
    void Draw(const int32_t renderTexture);
    void DrawImGui(const char* label);
    //利益の設定 
    void SetValue(const int32_t benefit)
    {
        startValue_ = value_;
        targetValue_ = benefit;
        isUpdateValue_ = true;
        timer_ = 0.0f;
    };


	void AddValue(const int32_t benefit)
	{
        startValue_ = value_;
        targetValue_ += benefit;
		isUpdateValue_ = true;
        timer_ = 0.0f;
	};

	int32_t GetValue() const { return targetValue_; }
    void SetIsEaseNum(const bool isEase) { isEase_ = isEase; }
    void SetEulerTransform(const EulerTransforms& transform);
    const Vector3 GetWorldPos();
private:
    bool isEase_ = false;
    bool isMinus_ = false;
    int32_t targetValue_ = 0;
    int32_t startValue_ = 0;
    float timer_ = 0.0f;
    int32_t value_ = 0;
    bool isUpdateValue_ = false;
    //6桁
    uint32_t maxDigit_ = 6;
    std::vector<std::unique_ptr<Numbers>>numbers_;
    std::unique_ptr<Numbers> minus_ = nullptr;
  
};

