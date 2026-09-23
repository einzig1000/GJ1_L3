#include "NumMeshs.h"
#include"../Numbers/Numbers.h"
#include<algorithm>

NumMeshs::NumMeshs()
{
    Numbers::Load();

    numbers_.resize(maxDigit_);

    for (uint32_t i = 0; i < maxDigit_; ++i) {
        numbers_[i] = std::make_unique<Numbers>();
    }
    //マイナス
    minus_ = std::make_unique<Numbers>();
}

NumMeshs::~NumMeshs()
{
}


void NumMeshs::Initialize(const uint32_t maxDigit, const EulerTransforms& transform, Matrix4x4* parent)
{
    maxDigit_ = maxDigit;
    for (uint32_t i = 0; i < maxDigit; ++i) {
        numbers_[i]->Initialize(0, transform.translate + Vector3{ i * transform.scale.x * 0.5f,0.0f,0.0f }, transform.rotate, transform.scale, parent);
    }

    //マイナスは10のインデックスに入っている
    minus_->Initialize(10, transform.translate - Vector3{ transform.scale.x * 0.5f,0.0f,0.0f }, transform.rotate, transform.scale, parent);

}

void NumMeshs::Update(const int32_t cameraID)
{
    isMinus_ = value_ < 0.0f;

    if (isUpdateValue_) {

        //カンスト処理
        value_ = std::clamp(value_, 0, 1000000 - 1);
        int32_t tempBenefit = std::abs(value_);

        for (int digit = maxDigit_ - 1; digit >= 0; digit--) {

            int digitNum = std::powf(10, digit);
            int num = tempBenefit / digitNum;
            numbers_[maxDigit_ - 1 - digit]->SetModelId(num);
            tempBenefit %= digitNum;
        }


    }

    Vector4 color = { 1.0f,1.0f,1.0f,1.0f };

    if (isMinus_) {
        color = { 1.0f,0.0f,0.0f,1.0f };
        minus_->SetColor(color);
        minus_->Update(cameraID);
    }

    for (uint32_t i = 0; i < maxDigit_; ++i) {
        numbers_[i]->SetColor(color);
        numbers_[i]->Update(cameraID);
    }


}

void NumMeshs::Draw(const int32_t renderTexture)
{
    for (uint32_t i = 0; i < maxDigit_; ++i) {
        numbers_[i]->Draw(renderTexture);
    }

    if (isMinus_) {
        minus_->Draw(renderTexture);
    }

}

void NumMeshs::DrawImGui(const char* label)
{

    ImGui::Begin("UI");

    if (ImGui::TreeNode(label)) {
        static int32_t value = 0;
        ImGui::SliderInt(label, &value, -999999, 999999);

        if (ImGui::Button("UpdateNumber")) {
            SetValue(value);
        }

        for (uint32_t i = 0; i < maxDigit_; ++i) {
            numbers_[i]->DrawImGui(label);
        }


        ImGui::TreePop();
    }

    ImGui::End();
}

void NumMeshs::SetEulerTransform(const EulerTransforms& transform)
{

    for (uint32_t i = 0; i < maxDigit_; ++i) {
        EulerTransforms numTransform = EulerTransforms{ .scale = transform.scale,.rotate = transform.rotate,.translate = {transform.translate + Vector3{ i * transform.scale.x * 0.5f,0.0f,0.0f }} };
        numbers_[i]->SetTransform(numTransform);
    }

    EulerTransforms minusTransform = EulerTransforms{ .scale = transform.scale,.rotate = transform.rotate, .translate = {transform.translate - Vector3{ transform.scale.x * 0.5f,0.0f,0.0f }} };
    //マイナスは10のインデックスに入っている
    minus_->SetTransform(minusTransform);

}

const Vector3 NumMeshs::GetWorldPos()
{
    int32_t center = maxDigit_ / 2;

    if (maxDigit_ % 2 == 0) {
        //偶数だったら
       return (numbers_[center-1]->WorldPos()+numbers_[center]->WorldPos())*0.5f;
    }

    return numbers_[center]->WorldPos();
}
