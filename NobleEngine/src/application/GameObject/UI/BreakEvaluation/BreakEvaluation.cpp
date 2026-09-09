#include "BreakEvaluation.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>
#include<numbers>

BreakEvaluation::BreakEvaluation()
{
    currentCountMesh_ = std::make_unique<NumMeshs>();

    breakSignboard_ = std::make_unique<UIModel>();

    evaluations_["Bad"] = std::make_unique<UIModel>();
    evaluations_["Good"] = std::make_unique<UIModel>();
    evaluations_["Great"] = std::make_unique<UIModel>();
    evaluations_["Parfect"] = std::make_unique<UIModel>();

    breakWord_ = std::make_unique<UIModel>();

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

    transformKey_["start"].transform_ = { { 0.0f ,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {-1.0f,-0.3f,-0.2f} };
    transformKey_["middle"].transform_ = { {1.2f ,1.2f,1.2f}, {0.0f,0.0f,0.0f}, {0.0f,-0.3f,-0.2f} };
    transformKey_["end"] .transform_ = { {0.0f ,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {1.0f,-0.3f,-0.2f} };

    transformKey_["start"].time = 0.0f;
    transformKey_["default"].time = 0.25f;
    transformKey_["middle"].time = 0.75f;
    transformKey_["end"].time = 1.0f;
}

BreakEvaluation::~BreakEvaluation()
{
}

void BreakEvaluation::Initialize()
{    //破壊数
    breakCount_ = 0;
    evaluationString_ = "unKnown";

    numberAniTime_ = {.isEnd = false,.timer = 0.0f};
    evalutionAniTime_ = { .isEnd = false,.timer = 0.0f };

    float pi = 3.14159265358979f;

    breakSignboard_->Initialize(
        modelIds_["breakSignboard"].model_,
        modelIds_["breakSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f }, { 0.0f,pi,0.0f },{ 0.0f,0.0f,0.0f } });

    breakWord_->Initialize(
        modelIds_["break"].model_,
        modelIds_["break"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f },  { 0.1f,0.0f,0.0f },{0.0f,0.7f,-0.2f } },
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Bad"]->Initialize(
        modelIds_["Bad"].model_,
        modelIds_["Bad"].texture_,
        transformKey_["start"].transform_,
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Good"]->Initialize(
        modelIds_["Good"].model_,
        modelIds_["Good"].texture_,
        transformKey_["start"].transform_,
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Great"]->Initialize(
        modelIds_["Great"].model_,
        modelIds_["Great"].texture_,
        transformKey_["start"].transform_,
        breakSignboard_->GetWorldMatrixPtr()
    );

    evaluations_["Parfect"]->Initialize(
        modelIds_["Parfect"].model_,
        modelIds_["Parfect"].texture_,
        transformKey_["start"].transform_,
        breakSignboard_->GetWorldMatrixPtr()
    );

    currentCountMesh_->Initialize(2, transformKey_["start"].transform_, breakSignboard_->GetWorldMatrixPtr());

}

void BreakEvaluation::Update(const int32_t uiCameraId)
{
    BreakJudgement();

    if (isAnimation_) {

        if (numberAniTime_.isEnd) {

           if (evaluationString_ != "unKnown") {
               evaluations_[evaluationString_]->SetEulerTransform(AnimationStart(evalutionAniTime_));
           }

        } else if(evalutionAniTime_.isEnd){
            //アニメーションの終了
            isAnimation_ = false;
       
        } else {
            currentCountMesh_->SetEulerTransform(AnimationStart(numberAniTime_));
        }
    }

    //それぞれの判定に対応したレンダーオブジェクトを更新する
    if (evaluationString_ != "unKnown") {
        evaluations_[evaluationString_]->Update(uiCameraId);
    }

    breakSignboard_->Update(uiCameraId);
    breakWord_->Update(uiCameraId);
    currentCountMesh_->Update(uiCameraId);

}

void BreakEvaluation::Draw(const int32_t renderTextureID)
{
    breakSignboard_->Draw(renderTextureID);

    if (evaluationString_ != "unKnown"&& numberAniTime_.isEnd) {
        //それぞれの判定に対応したレンダーオブジェクトを出力する
        evaluations_[evaluationString_]->Draw(renderTextureID);
    }
    breakWord_->Draw(renderTextureID);
    currentCountMesh_->Draw(renderTextureID);
}

void BreakEvaluation::DebugImGui()
{
    ImGui::Begin("UI");
    //破壊数
    ImGui::DragInt("breakCount", &breakCount_);
    ImGui::DragInt("maxBreakCount", &maxBreakCount_);

    if (ImGui::Button("AnimationStart")) {
        SetBreakCount(breakCount_);
    }

    ImGui::End();

    int i = 100;

    breakSignboard_->DebugUI(i++);

    breakWord_->DebugUI(i++);

    for (auto& [name, e] : evaluations_) {
        e->DebugUI(i++);
    }

    currentCountMesh_->DrawImGui("currentCountMesh");
}

void BreakEvaluation::SetBreakCount(const int32_t breakCount)
{
    isAnimation_ = true;
    //アニメーションが開始される
    numberAniTime_ = { .isEnd = false,.timer = 0.0f };
    evalutionAniTime_ = { .isEnd = false,.timer = 0.0f };

    breakCount_ = breakCount;
    currentCountMesh_->SetValue(breakCount_);
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

EulerTransforms BreakEvaluation::AnimationStart(AniTime& aniTime)
{
    //ここを後でスロー演出用に何とかする？
    aniTime.timer += Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    EulerTransforms transform;

    const float defaultTime = transformKey_["default"].time;
    const float middleTime = transformKey_["middle"].time;
    const float endTime = transformKey_["end"].time;

    if (aniTime.timer <= defaultTime) {

        float duration = defaultTime;
        float t = aniTime.timer / duration;

        transform = Easing(transformKey_["start"].transform_, transformKey_["middle"].transform_, EaseType::LINEAR, t);

    } else if (aniTime.timer <= middleTime) {
       
        transform = transformKey_["middle"].transform_;

    } else if (aniTime.timer <= endTime) {

        float duration = endTime - middleTime;
        float t = (aniTime.timer - middleTime) / duration;

        transform = Easing( transformKey_["middle"].transform_, transformKey_["end"].transform_, EaseType::LINEAR, t);
    } else {
        transform = transformKey_["end"].transform_;
        aniTime.timer = endTime;
        aniTime.isEnd = true;
    }

    return transform;
}

EulerTransforms BreakEvaluation::Easing(const EulerTransforms& start, const EulerTransforms& end, const EaseType type, float time)
{
    EulerTransforms result;
    result.scale = Game::Math::Ease::Easing(start.scale,end.scale,type,time);
    result.rotate = Game::Math::Ease::Easing(start.rotate, end.rotate, type, time);
    result.translate = Game::Math::Ease::Easing(start.translate, end.translate, type, time);

    return result;
}
