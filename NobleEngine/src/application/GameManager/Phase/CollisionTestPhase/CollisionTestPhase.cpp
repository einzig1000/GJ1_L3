#include "CollisionTestPhase.h"
#include"GameObject/Glass/Glass.h"

CollisionTestPhase::CollisionTestPhase()
{
    // カメラ
    c_main_ = Game::Camera::AddCamera("SimpleModels");
}

CollisionTestPhase::~CollisionTestPhase()
{
}

void CollisionTestPhase::Initialize()
{
    // フェーズ初期化
    nextPhase_ = Phase::Phase_None;

    InitGameObj();

}

void CollisionTestPhase::Update()
{
    Game::Camera::Update(c_main_);

    UpdateGameObj(c_main_);
}

void CollisionTestPhase::Draw()
{
    DrawGameObj();
}

void CollisionTestPhase::DrawImGui()
{
    DrawImGuiObj();
}

void CollisionTestPhase::InitGameObj()
{   //グラスの作成
    glass_ = std::make_unique<Glass>();
    glass_->Initialize();
}

void CollisionTestPhase::UpdateGameObj(const int32_t cameraID)
{
    glass_->Update(cameraID);
}

void CollisionTestPhase::DrawGameObj()
{
    glass_->Draw();
}

void CollisionTestPhase::DrawImGuiObj()
{
    glass_->DrawImGui();
}
