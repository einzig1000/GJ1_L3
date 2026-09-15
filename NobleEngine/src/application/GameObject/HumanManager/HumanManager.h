#pragma once
#include<Game.h>
class HumanModel;
class Glass;
class Table;

class HumanManager
{
public:
    HumanManager();
    ~HumanManager();
    void SetLightData(LightDataForGPU* data);
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw(const int32_t renderTexture3D);
    void DrawImGui(Glass* glass, Table* table);
    float GetCurrentGlaassUserDegree() { return  humanRotateDegree[currentGlassUserIndex_]; }
    void SetCurrentGlassUserIndex(const int32_t index) { currentGlassUserIndex_ = index; }
    void SetIsShotPtr(bool* isShotPtr);
    bool Load(const std::string path, const int32_t stage,const Vector3 tableCenter,const float tableRadius);
    void Save(const std::string path, const int32_t stage);
    float GetMarkerAngle(int32_t index);
private:
    bool* isShotPtr_ = nullptr;
    // 人間
    std::unique_ptr<HumanModel> human_[3];
    float humanRotateDegree[3] = { 90.0f, 210.0f, 330.0f, };	// 人間がテーブルから見てどの角度にいるか
    int32_t currentGlassUserIndex_ = 0;					// 現在グラスを持っている人間のインデックス
    float humanCatchSize_ = 30.0f;							// キャッチ出来る角度

    float customerRotateDegree_ = 0.0f;

    //お客さん
    std::unique_ptr<HumanModel> customer_ = nullptr;

    // マーカー(デバッグ描画)
    std::unique_ptr<RenderObject> markers_[6];
    EulerTransforms markerTransforms_[6];
    std::vector<float> markerAngles_;					// マーカーがテーブルから見てどの角度にいるか

};

