#include "HumanManager.h"
#include <GameObject/HumanModel/HumanModel.h>
#include <GameObject/Bartender/Bartender.h>
#include <GameObject/Customer/Customer.h>
#include <Utilities/Json/JsonManager.h>

#include <GameObject/Table/Table.h>
#include <GameObject/Glass/Glass.h>
#include<System/GameFunction/GameFunction.h>
#include<numbers>
#include<Utilities/Logger/Logger.h>


HumanManager::HumanManager()
{
    markerAngles_.resize(6);
    for (int32_t i = 0; i < 6; i++)
    {
        markers_[i] = std::make_unique<RenderObject>();
        markers_[i]->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
        markers_[i]->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
        markers_[i]->SetupFromShaders();
        markers_[i]->modelID_ = Game::Asset::Model::Load("assets/engine/model/cube/cube.obj");
    }

    for (int32_t i = 0; i < 3; i++)
    {
        //バーテンダー
        human_[i] = std::make_unique<Bartender>();
        human_[i]->Load();
    }

    customer_ = std::make_unique<Customer>();
    customer_->Load();
}

HumanManager::~HumanManager()
{
}

void HumanManager::SetLightData(LightDataForGPU* data)
{
    for (auto& human : human_) {
        human->SetLightData(data);
    }

    customer_->SetLightData(data);
}

void HumanManager::Initialize()
{
    for (auto& human : human_) {
        human->Initialize();
    }

    customer_->Initialize();
}

void HumanManager::Update(const int32_t cameraID)
{
    //現在の所有しているインデックスを得て発射したらショットアニメーションをする
    if (isShotPtr_ && *isShotPtr_) {
        human_[currentGlassUserIndex_]->SetAnimation("ShotGrass");
    }

    for (auto& human : human_) {
        human->Update(cameraID);
    }

    customer_->Update(cameraID);
}

void HumanManager::Draw(const int32_t renderTexture3D)
{
    for (auto& human : human_) {
        human->Draw(renderTexture3D);
    }

    customer_->Draw(renderTexture3D);
}

bool HumanManager::DrawImGui(const Vector3& tableCenter, const float tableRadius)
{

    for (int32_t i = 0; i < 3; i++) {
        human_[i]->DrawImGui(i);
    }

    customer_->DrawImGui(4);

    ImGui::Begin("Editor");

    bool edit = false;

    // 人間の位置
    if (ImGui::TreeNode("Human"))
    {

        if (ImGui::DragFloat("CustomerRotateDegree", &customerRotateDegree_, 1.0f, -360.0f, 720.0f)) edit = true;
        if (ImGui::DragFloat3("HumanRotate", humanRotateDegree, 1.0f, -360.0f, 720.0f)) edit = true;
        if (ImGui::DragFloat("HumanScale", &humanCatchSize_, 1.0f, 0.0f, 120.0f)) edit = true;


        if (edit)
        {

            for (int32_t i = 0; i < 3; i++)
            {
                Vector3 humanPos = GameFunction::GetPositionOnCircle(tableCenter, tableRadius * 1.2f, humanRotateDegree[i]);
                humanPos.y = 1.28f;
                human_[i]->SetTranslate(humanPos);
            }


            for (int32_t i = 0; i < 6; i++)
            {
                float hugou = i % 2 == 0 ? -1.0f : 1.0f;
                float angle = humanRotateDegree[(i / 2)] + (hugou * humanCatchSize_ * 0.5f);
                markerAngles_[i] = angle;
            }


            for (int32_t i = 0; i < 6; i++)
            {
                markerTransforms_[i].translate = GameFunction::GetPositionOnCircle(tableCenter, tableRadius * 0.8f, markerAngles_[i]);
                markerTransforms_[i].translate.y = 1.28f;
                markerTransforms_[i].scale = Vector3{ 0.1f,0.1f,0.1f };
            }


        }

        ImGui::TreePop();
    }

    ImGui::End();

    return edit;


}

float HumanManager::GetHumanRotateDegree(const int32_t index)
{
    if (index > 2) {
        Log("ここまで来たら角度の範囲外参照しようとしているよ");
        return 0.0f;
    }

    return humanRotateDegree[index];
}

void HumanManager::SetIsShotPtr(bool* isShotPtr)
{
    isShotPtr_ = isShotPtr;
    HumanModel::SetIsShotPtr(isShotPtr);
}

bool HumanManager::Load(const std::string path, const int32_t stage, const Vector3& tableCenter, const float tableRadius)
{

    std::string key = "/Stage" + std::to_string(stage) + "/Human/RotateDeg";
    Vector3 humanRotateDeg;
    bool success = JsonManager::Load(path, key, humanRotateDeg);
    if (!success) return false;
    humanRotateDegree[0] = humanRotateDeg.x;
    humanRotateDegree[1] = humanRotateDeg.y;
    humanRotateDegree[2] = humanRotateDeg.z;
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";

    const float pi2point5 = std::numbers::pi_v<float>*2.5f;
    for (int32_t i = 0; i < 3; i++)
    {
        Vector3 humanPos = GameFunction::GetPositionOnCircle(tableCenter, tableRadius * 0.8f, humanRotateDegree[i]);
        humanPos.y = 0.0f;
        human_[i]->SetTranslate(humanPos);
        human_[i]->SetRotateY(-humanRotateDegree[i] * (std::numbers::pi_v<float> / 180.0f) + pi2point5);
    }

    success = JsonManager::Load(path, key, humanCatchSize_);
    if (!success) return false;

    for (int32_t i = 0; i < 6; i++)
    {
        float hugou = i % 2 == 0 ? -1.0f : 1.0f;
        float angle = humanRotateDegree[(i / 2)] + (hugou * humanCatchSize_ * 0.5f);
        markerAngles_[i] = angle;
    }
    for (int32_t i = 0; i < 6; i++)
    {
        markerTransforms_[i].translate = GameFunction::GetPositionOnCircle(tableCenter, tableRadius * 0.8f, markerAngles_[i]);
        markerTransforms_[i].translate.y = 1.28f;
        markerTransforms_[i].scale = Vector3{ 0.1f,0.1f,0.1f };
    }

    //お客さんの位置を取得する
    key = "/Stage" + std::to_string(stage) + "/Customer/RotateDeg";
    success = JsonManager::Load(path, key, customerRotateDegree_);
    if (!success) return false;

    Vector3 customerTranslete = GameFunction::GetPositionOnCircle(tableCenter, tableRadius * 0.8f, customerRotateDegree_);
    //注意　一律ここで設定する
    customerTranslete.y = 0.5f;
    customer_->SetTranslate(customerTranslete);

    customer_->SetRotateY(-customerRotateDegree_ * (std::numbers::pi_v<float> / 180.0f) + pi2point5);

    return true;
}

void HumanManager::Save(const std::string path, const int32_t stage)
{
    std::string key = "/Stage" + std::to_string(stage) + "/Human/RotateDeg";
    Vector3 humanRotateDeg = { humanRotateDegree[0], humanRotateDegree[1], humanRotateDegree[2] };
    JsonManager::AddParam(path, key, humanRotateDeg);
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";
    JsonManager::AddParam(path, key, humanCatchSize_);
    key = "/Stage" + std::to_string(stage) + "/Customer/RotateDeg";
    JsonManager::AddParam(path, key, customerRotateDegree_);

}

float HumanManager::GetMarkerAngle(int32_t index)
{
    //安全処理
    if (index < markerAngles_.size()) {
        return markerAngles_[index];
    }
    //ここまで来たら0を返す
    Log("ここまで来たらマーカーアングルが範囲外参照しようとしているよ");
    return 0.0f;

}
