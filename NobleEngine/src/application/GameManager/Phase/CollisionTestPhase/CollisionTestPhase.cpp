#include "CollisionTestPhase.h"
#include"GameObject/Glass/Glass.h"
#include"GameObject/Table/Table.h"
//人間モデル
#include"GameObject/HumanModel/HumanModel.h"
#include"../System/CollisionManager/CollisionManager.h"

CollisionTestPhase::CollisionTestPhase()
{
    // カメラ
    c_main_ = Game::Camera::AddCamera("SimpleModels");
}

CollisionTestPhase::~CollisionTestPhase()
{
    collisionManager_->Finalize();
}

void CollisionTestPhase::Initialize()
{
    // フェーズ初期化
    nextPhase_ = Phase::Phase_None;
    //コリジョン管理
    collisionManager_ = std::make_unique<CollisionManager>();
    collisionManager_->Load();

    InitGameObj();

}

void CollisionTestPhase::Update()
{
    Game::Camera::Update(c_main_);

    UpdateGameObj(c_main_);

    CheckColliders();
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
{
    //グラスの作成
    glass_ = std::make_unique<Glass>();
    glass_->Initialize();
    //テーブルの作成
    table_ = std::make_unique<Table>();
    table_->Initialize();
    //人間モデル
      humanModel_ = std::make_unique<HumanModel>();
      humanModel_->Initialize();
}

void CollisionTestPhase::UpdateGameObj(const int32_t cameraID)
{
    glass_->Update(cameraID);
    table_->Update(cameraID);
    humanModel_->Update(cameraID);

    //コライダー描画のための更新
    collisionManager_->DebugUpdate(cameraID);


}

void CollisionTestPhase::DrawGameObj()
{

    //テーブルの描画
    table_->Draw();
    //人間モデル
    humanModel_->Draw();
    //グラスは半透明なので後に描画する
    glass_->Draw();

    //コライダーデバック描画
    collisionManager_->DebugDraw();

}

void CollisionTestPhase::DrawImGuiObj()
{
    glass_->DrawImGui();
    table_->DrawImGui();
    humanModel_->DrawImGui();
    collisionManager_->DebugImGui();
}

void CollisionTestPhase::CheckColliders()
{
    //コライダーリストを毎フレーム削除してみる？
    collisionManager_->ClearColliders();

    //コライダーを追加する
    for (auto& collider : glass_->GetColliders()) {
     collisionManager_->AddCollider(collider.get());
    }
    for (auto& collider : table_->GetColliders()) {
        collisionManager_->AddCollider(collider.get());
    }
    //人間
    for (auto& collider : humanModel_->GetColliders()) {
        collisionManager_->AddCollider(collider.get());
    }

   
    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}
