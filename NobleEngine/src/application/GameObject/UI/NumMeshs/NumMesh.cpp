#include "NumMeshs.h"
#include"../Numbers/Numbers.h"

NumMeshs::NumMeshs()
{    
    Numbers::Load();
    numbers_.resize(maxDigit);
    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i] = std::make_unique<Numbers>();
    }
    //マイナス
    minus_ = std::make_unique<Numbers>();
}

NumMeshs::~NumMeshs()
{
}

void NumMeshs::Initialize(const uint32_t maxDigit, const Vector3& startPos, const Vector3& rotate, const Vector3& scale)
{
    //最初はマイナス値にしてみる
    isMinus_ = true;

    for (int i = 0; i < maxDigit; ++i) {

        numbers_[i]->Initialize(0, startPos + Vector3{ i * scale.x * 0.5f,0.0f,0.0f }, rotate, scale);
    }

    //マイナスは10のインデックスに入っている
    minus_->Initialize(10, startPos - Vector3{ scale.x*0.5f,0.0f,0.0f }, rotate, scale);
}

void NumMeshs::Update(const int32_t cameraID)
{

    if (isUpdateValue_) {

        isMinus_ = value_ < 0.0f;

        int32_t tempBenefit = std::abs(value_);

        for (int digit = maxDigit-1; digit >= 0; digit--) {
          
            int digitNum = std::powf(10, digit);
            int num = tempBenefit / digitNum;
            numbers_[maxDigit - 1-digit]->SetModelId(num);
            tempBenefit %= digitNum;
        }
    }

    Vector4 color = { 1.0f,1.0f,1.0f,1.0f };

    if (isMinus_) {
       color = { 1.0f,0.0f,0.0f,1.0f };
       minus_->SetColor(color);
       minus_->Update(cameraID);
    }

    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i]->SetColor(color);
        numbers_[i]->Update(cameraID);
    }


}

void NumMeshs::Draw()
{
    for (int i = 0; i < maxDigit; ++i) {
        numbers_[i]->Draw();
    }

    if (isMinus_) {
        minus_->Draw();
    }

}

void NumMeshs::DrawImGui()
{

    ImGui::Begin("UI");

    static int32_t benefit = 0;
    ImGui::SliderInt("benefit", &benefit, -999999, 999999);
    
    if (ImGui::Button("UpdateBenefit")) {
        SetValue(benefit);
    }

    ImGui::End();
}
