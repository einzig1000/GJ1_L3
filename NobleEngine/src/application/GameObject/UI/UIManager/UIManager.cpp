#include "UIManager.h"
#include<GameObject/UI/NumMeshs/NumMeshs.h>
#include<GameObject/UI/UIModel/UIModel.h>
#include<numbers>

UIManager::UIManager()
{
    // カメラ
    uiCameraID_ = Game::Camera::AddCamera("UICamera");

    benefitMesh_ = std::make_unique<NumMeshs>();
    timeMesh_ = std::make_unique<NumMeshs>();
    breakCountMesh_ = std::make_unique<NumMeshs>();
    timeAndMoneySignboard_ = std::make_unique<UIModel>();
    breakSignboard_ = std::make_unique<UIModel>();
    cockTailSignboard_ = std::make_unique<UIModel>();

    evaluations_["Bad"]  = std::make_unique<UIModel>();
    evaluations_["Good"] = std::make_unique<UIModel>();
    evaluations_["Great"] = std::make_unique<UIModel>();
    evaluations_["Parfect"] = std::make_unique<UIModel>();

    yenWorl_ = std::make_unique<UIModel>();
    timeWord_ = std::make_unique<UIModel>();
    breakWorl_ = std::make_unique<UIModel>();


    modelIds_["timeAndMoney"] = {
     Game::Asset::Model::Load("assets/application/model/UI_TIme&Money/UI_TIme&Money.obj") ,
     Game::Asset::Texture::Load("assets/application/model/UI_TIme&Money/UI.png")
    };

    modelIds_["breakSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_break/UI_break.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_break//UI.png")
    };


    modelIds_["cocktailSignboard"] = {
    Game::Asset::Model::Load("assets/application/model/UI_cocktail/UI_cocktail.obj") ,
    Game::Asset::Texture::Load("assets/application/model/UI_cocktail/UI.png")
    };

    int32_t white1x1 = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");

    modelIds_["yen"] = {
    Game::Asset::Model::Load("assets/application/model/Yen/yen.obj") ,
    white1x1
    };


    modelIds_["time"] = {
    Game::Asset::Model::Load("assets/application/model/Time/time.obj") ,
    white1x1
    };

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

    const uint32_t windowWidthSize =Game::Window::GetWidth();
    const uint32_t windowHeightSize = Game::Window::GetHeight();

    renderTextureID_ = Game::Asset::RenderTexture::CreateRenderTexture(Game::Window::GetWidth(), Game::Window::GetHeight(),"UIRender");
}

UIManager::~UIManager()
{

}

void UIManager::Initialize()
{
    //破壊数
    breakCount_ = 0;
    evaluationString_ = "unKnown";

    float pi = 3.14159265358979f;
    float range = 8.0f;

    timeAndMoneySignboard_->Initialize(
        modelIds_["timeAndMoney"].model_,
        modelIds_["timeAndMoney"].texture_,
        EulerTransforms{ {1.0f,1.0f,1.0f} ,{ 0.0f,3.642f,0.0f }, { -range,0.0f,1.0f } });

    breakSignboard_->Initialize(
        modelIds_["breakSignboard"].model_,
        modelIds_["breakSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f }, { 0.0f,pi,0.0f },{ 0.0f,0.0f,0.0f } });

    cockTailSignboard_->Initialize(
        modelIds_["cocktailSignboard"].model_,
        modelIds_["cocktailSignboard"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,2.562f,0.0f }, { range,0.0f,1.0f } });

    yenWorl_->Initialize(
        modelIds_["yen"].model_,
        modelIds_["yen"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f,0.0f,0.0f },{-1.67f,0.4f,-0.2f } },
        timeAndMoneySignboard_->GetWorldMatrixPtr()
    );

    timeWord_->Initialize(
        modelIds_["time"].model_,
        modelIds_["time"].texture_,
        EulerTransforms{ { 1.0f,1.0f,1.0f } , { 0.0f, 0.0f,0.0f },{ -1.32f,-0.63f,-0.2f} },
        timeAndMoneySignboard_->GetWorldMatrixPtr()
    );

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
        EulerTransforms{ { 1.0f,1.0f,1.0f },  {- 1.57f,0.0f,0.0f },{-1.0f,-0.36f,-0.2f } },
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

    benefitMesh_->Initialize(6, { { 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, {-0.28f,0.4f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    timeMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, { 0.8f,-0.63f,-0.2f } }, timeAndMoneySignboard_->GetWorldMatrixPtr());
    breakCountMesh_->Initialize(2, { { 1.1f ,1.1f,1.1f }, { 0.0f,0.0f,0.0f }, {0.5f,-0.3f,-0.2f } }, breakSignboard_->GetWorldMatrixPtr());
}

void UIManager::Update()
{
    //Game::Camera::Setter::SetCenter();
    Game::Camera::Update(uiCameraID_);

    BreakJudgement();

    //それぞれの判定に対応したレンダーオブジェクトを更新する
    if (evaluationString_ != "unKnown") {
        evaluations_[evaluationString_]->Update(uiCameraID_);
    }

    timeAndMoneySignboard_->Update(uiCameraID_);
    breakSignboard_->Update(uiCameraID_);
    cockTailSignboard_->Update(uiCameraID_);

    yenWorl_->Update(uiCameraID_);
    timeWord_->Update(uiCameraID_);
    breakWorl_->Update(uiCameraID_);

    benefitMesh_->Update(uiCameraID_);
    timeMesh_->Update(uiCameraID_);
    breakCountMesh_->Update(uiCameraID_);
}

void UIManager::Draw()
{
 
    timeAndMoneySignboard_->Draw(renderTextureID_);
    breakSignboard_->Draw(renderTextureID_);
    cockTailSignboard_->Draw(renderTextureID_);
    
    if (evaluationString_ != "unKnown") {
        //それぞれの判定に対応したレンダーオブジェクトを出力する
        evaluations_[evaluationString_]->Draw(renderTextureID_);
    }

    yenWorl_->Draw(renderTextureID_);
    timeWord_->Draw(renderTextureID_);
    breakWorl_->Draw(renderTextureID_);
    benefitMesh_->Draw(renderTextureID_);
    timeMesh_->Draw(renderTextureID_);
    breakCountMesh_->Draw(renderTextureID_);
}

void UIManager::DebugImGui()
{

    ImGui::Begin("UI");
    //破壊数
    ImGui::DragInt("breakCount", &breakCount_);
    ImGui::DragInt("maxBreakCount", &maxBreakCount_);

    ImGui::End();

    int i = 0;
    timeAndMoneySignboard_->DebugUI(i++);
    breakSignboard_->DebugUI(i++);
    cockTailSignboard_->DebugUI(i++);
    yenWorl_->DebugUI(i++);
    timeWord_->DebugUI(i++);
    breakWorl_->DebugUI(i++);

    for (auto& [name,e] : evaluations_) {
        e->DebugUI(i++);
    }

    benefitMesh_->DrawImGui("benefit");
    timeMesh_->DrawImGui("timeMesh");
    breakCountMesh_->DrawImGui("breakCountMesh");
}

void UIManager::SetBreakCount(const int32_t breakCount)
{
    breakCount_ = breakCount;
}

void UIManager::SetMaxBreakCount(const int32_t maxBreakCount)
{
    maxBreakCount_ = maxBreakCount;
}

void UIManager::BreakJudgement()
{

    if (maxBreakCount_ == 0) {
        evaluationString_ = "unKnown";
        return;
    }
    if (breakCount_ <= 0) {
        //割ってしまう　一つも割れない
        evaluationString_ = "Bad";
    } else if (breakCount_ /static_cast<float>( maxBreakCount_) <= 0.7f) {
        //割った数が全体の70％以下
        evaluationString_ = "Good";
    } else if (breakCount_ / static_cast<float>(maxBreakCount_) < 1.0f) {
        //100％未満
        evaluationString_ = "Great";
    } else {
        evaluationString_ = "Parfect";
    }

}
