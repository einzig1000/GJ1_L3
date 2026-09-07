#pragma once
#include<memory>
#include<vector>

class Numbers;
class Benefit
{
public:
    Benefit();
    ~Benefit();
    void Initialize();
    void Update(const int32_t cameraID);
    void Draw();
    void DrawImGui();
    //利益の設定 
    void SetBenefit(const int32_t benefit) { 
    benefit_ = benefit;
    isUpdateBenefit_ = true;
     };

private:
    int32_t benefit_ = 0;
    bool isUpdateBenefit_ = false;
    //6桁
    const uint32_t maxDigit = 6;
    std::vector<std::unique_ptr<Numbers>>numbers_;
};

