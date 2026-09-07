#include "Benefit.h"
#include"../Numbers/Numbers.h"
Benefit::Benefit()
{
    
    Numbers::Load();

    numbers_.resize(maxDigit);
    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i] = std::make_unique<Numbers>();
    }
}

Benefit::~Benefit()
{
}

void Benefit::Initialize()
{
    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i]->Initialize(0, { i * 0.5f,0.0f,0.0f });
    }
}

void Benefit::Update(const int32_t cameraID)
{

    if (isUpdateBenefit_) {

        int32_t tempBenefit = benefit_;

        for (int digit = maxDigit-1; digit >= 0; digit--) {
            int digitNum = std::powf(10, digit);
            int num = tempBenefit / digitNum;
            numbers_[maxDigit-1-digit]->SetModelId(num);
            tempBenefit %= digitNum;
        }
    }

    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i]->Update(cameraID);
    }
}

void Benefit::Draw()
{
    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i]->Draw();
    }
}

void Benefit::DrawImGui()
{

    ImGui::Begin("UI");

    static int32_t benefit = 0;
    ImGui::SliderInt("benefit", &benefit, 0, 999999);
    
    if (ImGui::Button("UpdateBenefit")) {
        SetBenefit(benefit);
    }

    ImGui::End();
}
