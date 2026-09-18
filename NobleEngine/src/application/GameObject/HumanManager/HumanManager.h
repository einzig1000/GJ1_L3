#pragma once
#include<Game.h>
class HumanModel;

class Customer;

class HumanManager
{
public:
    HumanManager();
    ~HumanManager();
    void SetIsShotPtr(bool* isShotPtr);
    void SetLightData(LightDataForGPU* data);
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(const int32_t renderTexture3D);
    /// @brief 編集したかどうかを得る
    bool DrawImGui(const Vector3& tableCenter, const float tableRadius);
    float GetCurrentGlassUserDegree() { return  humanRotateDegree[currentGlassUserIndex_]; }
    float GetHumanRotateDegree(const int32_t index);
    void SetCurrentGlassUserIndex(const int32_t index) { currentGlassUserIndex_ = index; }

    bool Load(const std::string path, const int32_t stage,const Vector3& tableCenter,const float tableRadius);
    void Save(const std::string path, const int32_t stage);
    float GetMarkerAngle(int32_t index);
    Customer* GetCustomerPtr() { return customer_.get(); }
private:
    //借り物
    bool* isShotPtr_ = nullptr;

    // 人間
    std::unique_ptr<HumanModel> human_[3];
    float humanRotateDegree[3] = { 90.0f, 210.0f, 330.0f, };	// 人間がテーブルから見てどの角度にいるか
    int32_t currentGlassUserIndex_ = 0;					// 現在グラスを持っている人間のインデックス
    float humanCatchSize_ = 30.0f;							// キャッチ出来る角度

    float customerRotateDegree_ = 0.0f;

    //お客さん
    std::unique_ptr<Customer> customer_ = nullptr;

    // マーカー(デバッグ描画)
    std::unique_ptr<RenderObject> markers_[6];
    EulerTransforms markerTransforms_[6];
    std::vector<float> markerAngles_;					// マーカーがテーブルから見てどの角度にいるか

};

