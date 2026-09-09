#include "BreakEvaluation.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>

BreakEvaluation::BreakEvaluation()
{
    breakCountMesh_ = std::make_unique<NumMeshs>();
    breakSignboard_ = std::make_unique<UIModel>();

    evaluations_["Bad"] = std::make_unique<UIModel>();
    evaluations_["Good"] = std::make_unique<UIModel>();
    evaluations_["Great"] = std::make_unique<UIModel>();
    evaluations_["Parfect"] = std::make_unique<UIModel>();

    breakWorl_ = std::make_unique<UIModel>();

    modelIds_["breakSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_break/UI_break.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_break//UI.png")
    };

    int32_t white1x1 = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");


    modelIds_["break"] = {
  Game::Asset::Model::Load("assets/application/model/Break/break.obj") ,
  white1x1
    };

    // ========================//破壊の判定！//============================
    modelIds_["Bad"] = {
    Game::Asset::Model::Load("assets/application/model/Evaluation_UI/Bad.obj") ,
    white1x1
    };
    modelIds_["Good"] = {
    Game::Asset::Model::Load("assets/application/model/Evaluation_UI/Good.obj") ,
    white1x1
    };
    modelIds_["Great"] = {
    Game::Asset::Model::Load("assets/application/model/Evaluation_UI/Great.obj") ,
    white1x1
    };
    modelIds_["Parfect"] = {
    Game::Asset::Model::Load("assets/application/model/Evaluation_UI/Parfect.obj") ,
    white1x1
    };
}

BreakEvaluation::~BreakEvaluation()
{
}

void BreakEvaluation::Initialize()
{    //破壊数
    breakCount_ = 0;
    evaluationString_ = "unKnown";

    float pi = 3.14159265358979f;

    breakSignboard_->Initialize(
        modelIds_["breakSignboard"].model_,
        modelIds_["breakSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f }, { 0.0f,pi,0.0f },{ 0.0f,0.0f,0.0f } });

    breakWorl_->Initialize(
        modelIds_["break"].model_,
        modelIds_["break"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  { 0.1f,0.0f,0.0f },{0.0f,0.7f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Bad"]->Initialize(
        modelIds_["Bad"].model_,
        modelIds_["Bad"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  { -1.57f,0.0f,0.0f },{-1.0f,-0.36f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Good"]->Initialize(
        modelIds_["Good"].model_,
        modelIds_["Good"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  {-1.57f,0.0f,0.0f },{-1.0f,-0.36f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Great"]->Initialize(
        modelIds_["Great"].model_,
        modelIds_["Great"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  { -1.57f,0.0f,0.0f },{-1.0f,-0.36f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Parfect"]->Initialize(
        modelIds_["Parfect"].model_,
        modelIds_["Parfect"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  {-1.57f,0.0f,0.0f },{-1.0f,-0.36f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    breakCountMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, {0.5f,-0.3f,-0.2f } }, breakSignboard_->GetWorldMatrixPtr());

}

void BreakEvaluation::Update(const int32_t uiCameraId)
{
    BreakJudgement();

    //それぞれの判定に対応したレンダーオブジェクトを更新する
    if (evaluationString_ != "unKnown") {
        evaluations_[evaluationString_]->Update(uiCameraId);
    }

    breakSignboard_->Update(uiCameraId);
    breakWorl_->Update(uiCameraId);
    breakCountMesh_->Update(uiCameraId);
}

void BreakEvaluation::Draw(const int32_t renderTextureID)
{
    breakSignboard_->Draw(renderTextureID);

    if (evaluationString_ != "unKnown") {
        //それぞれの判定に対応したレンダーオブジェクトを出力する
        evaluations_[evaluationString_]->Draw(renderTextureID);
    }
    breakWorl_->Draw(renderTextureID);
    breakCountMesh_->Draw(renderTextureID);
}

void BreakEvaluation::DebugImGui()
{
    ImGui::Begin("UI");
    //破壊数
    ImGui::DragInt("breakCount", &breakCount_);
    ImGui::DragInt("maxBreakCount", &maxBreakCount_);

    ImGui::End();

    //breakSignboard_->DebugUI(i++);
    //breakWorl_->DebugUI(i++);
    //for (auto& [name, e] : evaluations_) {
    //    e->DebugUI(i++);
    //}

    breakCountMesh_->DrawImGui("breakCountMesh");
}

void BreakEvaluation::SetBreakCount(const int32_t breakCount)
{
    breakCount_ = breakCount;
}

void BreakEvaluation::SetMaxBreakCount(const int32_t maxBreakCount)
{
    maxBreakCount_ = maxBreakCount;
}

void BreakEvaluation::BreakJudgement()
{
    if (maxBreakCount_ == 0) {
        evaluationString_ = "unKnown";
        return;
    }
    if (breakCount_ <= 0) {
        //割ってしまう　一つも割れない
        evaluationString_ = "Bad";
    } else if (breakCount_ / static_cast<float>(maxBreakCount_) <= 0.7f) {
        //割った数が全体の70％以下
        evaluationString_ = "Good";
    } else if (breakCount_ / static_cast<float>(maxBreakCount_) < 1.0f) {
        //100％未満
        evaluationString_ = "Great";
    } else {
        evaluationString_ = "Parfect";
    }
}
