#include "GameScenePhase.h"
//テーブル
#include <GameObject/Table/Table.h>
//テーブル上の障害物
#include <GameObject/TableObject/TableObject.h>
//グラス総括管理
#include<GameObject/GlassManager/GlassManager.h>
#include<GameObject/Glass/Glass.h>
//グラスパーティクル
#include"GameObject/Effect/GlassParticle/GlassParticle.h"
//人間管理
#include<GameObject/HumanManager/HumanManager.h>
#include<GameObject/Customer/Customer.h>
//背景のバー
#include<GameObject/Bar/Bar.h>
//UI
#include <GameObject/UI/UIManager/UIManager.h>
//破壊数カウント
#include <GameObject/UI/BreakEvaluation/BreakEvaluation.h>
//ゲームライト
#include<GameObject/GameLight/GameLight.h>
//ゲームカメラ
#include<GameObject/GameCameraManager/GameCameraManager.h>
//当たり判定管理システム
#include <System/CollisionManager/CollisionManager.h>
//ゲームに使用する関数
#include<System/GameFunction/GameFunction.h>
//ゲーム画面
#include<GameObject/GameScreen/GameScreen.h>
//Json
#include <Utilities/Json/JsonManager.h>

#include<System/GameBGMSystem/GameBGMSystem.h>
#include<System/SESystem/GameSESystem/GameSESystem.h>

#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>

GameScenePhase::GameScenePhase()
{
    //ゲーム画面
    gameScreen_ = std::make_unique<GameScreen>();

    renderTargetID_ = Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(),
        Game::Window::GetHeight(),
        "gameScene",
        Vector4{ 0.0f,0.0f,0.0f,0.0f }
    );

    //UI管理
    uiManager_ = std::make_unique<UIManager>();
    //カメラ管理
    gameCameraManager_ = std::make_unique<GameCameraManager>();
    //ゲームライトの実体生成 ここで管理する
    gameLight_ = std::make_unique<GameLight>();

    //コリジョン管理
    collisionManager_ = std::make_unique<CollisionManager>();
    collisionManager_->SetTag();

    // オブジェクト実体生成
    table_ = std::make_unique<Table>();
    table_->SetLightData(&gameLight_->GetLightData());

    //グラス総括管理
    glassManager_ = std::make_unique<GlassManager>();
    glassManager_->SetLightData(&gameLight_->GetLightData());
    glassManager_->SetCollisionManager(collisionManager_.get());

    //人間管理
    humanManager_ = std::make_unique<HumanManager>();
    humanManager_->SetLightData(&gameLight_->GetLightData());
    //ショットアドレスを入れる
    humanManager_->SetIsShotPtr(&glassManager_->IsShotAddress());

    //配置用に使うつもりの残骸　もしかしたら後で参考にするかも
    //simpleObstaclePlacementFlow_ = std::make_unique<SimpleObstaclePlacementFlow>();

    //バー
    bar_ = std::make_unique<Bar>();
    bar_->SetLightData(&gameLight_->GetLightData());

    //ロード
    GameSESystem::Load(GameSESystem::Game);
}

GameScenePhase::~GameScenePhase() {}

void GameScenePhase::Initialize()
{
    // ======================================
    // 次のフェーズ
    // ======================================
    nextPhase_ = Phase::Phase_None;

    // ======================================
    // レンダーターゲットの設定
    // ======================================
    context_->renderTargetIDs[static_cast<size_t>(Phase::Phase_GameScene)] = renderTargetID_;

    // ======================================
    // オブジェクト初期化
    // ======================================

    //グラス総括管理
    glassManager_->Initialize();
    //テーブル
    table_->Initialize();
    //人間管理
    humanManager_->Initialize();
    //バー初期化（今は中身なし）
    bar_->Initialize();

    GameBGMSystem::GetInstance().StopAllAudio();
    // 音の設定
    GameBGMSystem::GetInstance().Initialize(GameBGMSystem::GAME_BGM);

    // ======================================
    // UI管理 ゲームタイマーも中に入っている
    // ======================================

    uiManager_->Initialize();

    // ======================================
    // ロード
    // ======================================

    //ゲームライトのロード
    gameLight_->Load();

    //オブジェクトデータをセットする
    LoadObstacleData(0);

    // GlassParticle
    glassParticle_ = std::make_unique<GlassParticle>();
    glassParticle_->Initialize();

    // ======================================
    // ロード後のデータを使用しての設定
    // ======================================

    //カメラの初期化
    gameCameraManager_->Initialize(humanManager_->GetCurrentGlassUserDegree());
    gameCameraManager_->ChangeCameraPhase(CameraPhase::CatchFollowing);

    //グラスの位置を設定しているよ
    glassManager_->SetPosForTableAndHuman(table_->GetTranslate(), table_->GetRadius(), humanManager_->GetCurrentGlassUserDegree(), 1.28f);

}

void GameScenePhase::Update()
{
#ifdef _RELEASE

    //リリース版ならシーン切り替えする
    if (uiManager_->GetTimer() <= 0.0f) {
        //フェーズ
        nextPhase_ = Phase::Phase_Result;
    } else if(uiManager_->GetTimer() <= 1.0f){
        GameBGMSystem::GetInstance().DownVolume(GameBGMSystem::GAME_BGM);
    }
#endif

    //カメラの更新
    gameCameraManager_->Update();
    //BGMの更新
    UpdateBGM();
    //テーブル内の出来事
    InnerTableEvent();
    //UI管理
    uiManager_->Update();

    // ==============//入力操作//======================

    //リセットゲーム
    ResetGame();
    //プレイヤー操作
    PlayerControl();

    // ==================//オブジェクト更新//=============================

    glassManager_->Update(gameCameraManager_->GetCameraID());
    table_->Update(gameCameraManager_->GetCameraID());
    
    if (deleteIndex >= 0 && deleteIndex < static_cast<int32_t>(obstacles_.size()))
    {
        obstacles_[deleteIndex] = std::move(obstacles_.back());
        obstacles_.pop_back();
        deleteIndex = -1;
    } else
    {
        deleteIndex = -1; // 範囲外なら安全にリセット
    }


    for (auto& obstacle : obstacles_) {
        obstacle->Update(gameCameraManager_->GetCameraID());
    }

    
    glassParticle_->Update(gameCameraManager_->GetCameraID());

    //人間管理
    humanManager_->Update(gameCameraManager_->GetCameraID());
    //バーの更新
    bar_->Update(gameCameraManager_->GetCameraID());

    //毎フレーム偽にする
    glassManager_->SetIsShot(false);

    // ==================//オブジェクト同士の判定//=============================

    //カメラのフェーズ
    gameCameraManager_->UpdateCameraPhase(
        glassManager_->GetAbleDragAddress(),//ここだけアドレスで書き換える
        glassManager_->GetGlassPtr()->GetTranslate(),
        glassManager_->GetGlassPtr()->GetVelocity(),
        glassManager_->GetMouseInsensitivity()
    );

    bool isSetUp = gameCameraManager_->GetCameraPhase() == CameraPhase::ShotAngleSetup;
    //UIで破壊カウントの看板を上にする
    uiManager_->GetBreakEvaluation()->SetIsUp(isSetUp);

    for (int32_t i = 0; i < obstacles_.size(); ++i)
    {
        if (obstacles_[i]->IsBroken())
        {
            deleteIndex = i;
            //ここってブレーク必要かも？

            //パーティクルを出現させる 反発方向にセットする
            glassParticle_->SetEmitColor(obstacles_[i]->GetEmitColor());
            glassParticle_->Emit(obstacles_[i]->GetTranslate(), obstacles_[i]->GetVelocity());
            //シェイク値を加算する
            glassManager_->AddShakeValue(0.15f);
            uiManager_->SetShakeProgress(glassManager_->GetShakeValue());
            break;
        }
    }

    //コライダーの判定を開始する
    CheckColliders();
 
    //コライダー更新
    if (isDebugDraw_) collisionManager_->DebugUpdate(gameCameraManager_->GetCameraID());

    if (isLoadRequested_)
    {
        // 予約されたステージをロード
        LoadObstacleData(requestLoadStage_);
        // フラグを下ろす
        isLoadRequested_ = false;
    }

}

void GameScenePhase::Draw()
{
    //メイン画面描画
    DrawMainScreen(gameScreen_->GetRenderTextureID(GameScreen::MAIN_SCREEN));
    //UIなので一番最後に描画する
    uiManager_->Draw(gameScreen_->GetRenderTextureID(GameScreen::UI_SCREEN));
    //実際に見せるゲーム画面の描画
    gameScreen_->Draw(renderTargetID_);

}

void GameScenePhase::DrawImGui()
{

    //glass
    glassManager_->DrawImGui();

    //obstacles_[0]->DrawImGui();
        //バーティクルデバック表示
    glassParticle_->DebugImGui(0);

    table_->DrawImGui();

    uiManager_->DrawImGui();

    if (humanManager_->DrawImGui(table_->GetTranslate(), table_->GetRadius())) {
        //人間管理が編集したら
        glassManager_->SetPosForTableAndHuman(table_->GetTranslate(), table_->GetRadius(), humanManager_->GetHumanRotateDegree(0));
    };

    //バー
    bar_->DrawImGui();

    ImGui::Begin("Editor");

    ImGui::Checkbox("canControll", &canControll_);

    ImGui::Checkbox("DebugDraw", &isDebugDraw_);

    // editStage選択
    static int currentStage_ = 0;
    ImGui::Text("CurrentStage");
    if (ImGui::Button("-", ImVec2(20, 20)))
    {
        currentStage_--;
    }
    ImGui::SameLine();
    ImGui::DragInt("##CurrentStage", &currentStage_, 1, 0, 100);
    ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(20, 20)))
    {
        currentStage_++;
    }
    currentStage_ = std::clamp(currentStage_, 0, 100);


        // 該当箇所を変更
        if (ImGui::Button("Load", ImVec2(50, 20)))
        {
            // 即座にロードせず、リクエストだけ送る
            isLoadRequested_ = true;
            requestLoadStage_ = currentStage_;
        }

    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(50, 20)))
    {
        SaveObstacleData(currentStage_);
    }

    // 障害物の追加
    if (ImGui::TreeNode("Add Obstacle"))
    {
        auto names = magic_enum::enum_names<GlassType>();
        auto values = magic_enum::enum_values<GlassType>();

        size_t current = magic_enum::enum_index(glassType).value();

        // GlassType先頭の要素を除外して表示する
        if (ImGui::BeginCombo("GlassType", names[current].data()))
        {
            for (std::size_t i = 1; i < names.size(); i++)
            {
                bool selected = (current == static_cast<int>(i));
                if (ImGui::Selectable(names[i].data(), selected))
                {
                    current = static_cast<int>(i);
                    glassType = values[i];
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        static Vector2 position;
        ImGui::DragFloat2("Position", &position.x, 0.01f, -1.5f, 1.5f);

        if (ImGui::Button("Add"))
        {
            obstacles_.push_back(std::make_unique<TableObject>());
            obstacles_.back()->Initialize();
            obstacles_.back()->SetLightData(&gameLight_->GetLightData());
            obstacles_.back()->SetGlassTypeAndLoadModels(glassType);
            Vector3 pos = { position.x, 1.28f, position.y };
            obstacles_.back()->SetTranslate(pos);
        }

        ImGui::TreePop();
    }

    // 障害物リスト
    if (ImGui::TreeNode("List"))
    {
        for (size_t i = 0; i < obstacles_.size(); ++i)
        {
            ImGui::PushID(static_cast<int32_t>(i));
            if (ImGui::TreeNode("Obstacle", "Obstacle %d", static_cast<int32_t>(i)))
            {
                Vector3 pos = obstacles_[i]->GetTranslate();
                Vector2 pos2D = { pos.x, pos.z };

                if (ImGui::DragFloat2("Position", &pos2D.x, 0.01f, -1.5f, 1.5f))
                {
                    Vector3 newPos = { pos2D.x, pos.y, pos2D.y };
                    obstacles_[i]->SetTranslate(newPos);

                }

                if (ImGui::Button("Delete"))
                {
                    deleteIndex = static_cast<int32_t>(i);
                }

                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }


    gameLight_->DrawImGui();

    ImGui::End();


}

void GameScenePhase::CheckColliders()
{
    //コライダーリストを毎フレーム削除してみる？
    collisionManager_->ClearColliders();

    //コライダーを追加する
    for (auto& collider : glassManager_->GetGlassPtr()->GetColliders())
    {
        collisionManager_->AddCollider(collider.get());
    }

    for (auto& obstacle : obstacles_)
    {
        for (auto& collider : obstacle->GetColliders())
        {
            collisionManager_->AddCollider(collider.get());
        }
    }

    for (auto& collider : table_->GetColliders())
    {
        collisionManager_->AddCollider(collider.get());
    }

    //コライダーを追加する
    for (auto& collider : humanManager_->GetCustomerPtr()->GetColliders())
    {
        collisionManager_->AddCollider(collider.get());
    }

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}

void GameScenePhase::InnerTableEvent()
{
    // テーブル外判定
    Vector2 tablePos2D = Vector2(table_->GetTranslate().x, table_->GetTranslate().z);
    Vector2 glassPos2D = glassManager_->GetGlassPtr()->Get2DPos();

    //テーブルの内円とグラスの外円が接触しているか
    bool isTouchingInnerEdge = GameFunction::IsTouchingInnerEdge(
        tablePos2D,
        table_->GetRadius(),
        glassPos2D,
        glassManager_->GetGlassPtr()->GetRadius()
    );

    if (!isTouchingInnerEdge) {
        //テーブルにいなかったら早期リターン
        return;
    }

    if (glassManager_->GetGlassPtr()->GetIsHitCustomer()) {
        //お客さんにヒットしたとき
        
        //UIマネージャーに値を入れた後に初期化する
        uiManager_->SetShakeProgress(glassManager_->GetShakeValue());
        //フラグを初期化する。速度を0にする、など  //シェイク値をご破算
        glassManager_->Initialize();

        JudgeAndSetCameraAndLoad();
        //UI カスタマーにヒットした
        uiManager_->SetIsHitCustomer(true);
        //仮にスライドSEを入れる
        GameSESystem::PlaySE(GameSESystem::SLIDE);
       
    } else if (glassManager_->GetGlassPtr()->GetIsBroken()) {

        //フラグを初期化する。速度を0にする、など  //シェイク値をご破算
        glassManager_->Initialize();
        JudgeAndSetCameraAndLoad();

        glassManager_->SetShakeValue(0.0f);
        uiManager_->SetShakeProgress(glassManager_->GetShakeValue());
        //グラスがテーブルから離れた。
        uiManager_->GetBreakEvaluation()->SetIsGlassOutOfTable(true);

    } else {

        float angle = GameFunction::GetContactAngleDeg(tablePos2D, glassPos2D);

        for (int i = 0; i < 3; i++)
        {
            humanManager_->GetMarkerAngle(i * 2);

            //angleがfromDegからtoDegの範囲に入ったか
            if (GameFunction::IsInAngleRange(
                angle,
                humanManager_->GetMarkerAngle(i * 2),
                humanManager_->GetMarkerAngle(i * 2 + 1)

            ))
            {

                //現在のグラスユーザーをセットする
                humanManager_->SetCurrentGlassUserIndex(i);
                //速度を初期化する
                glassManager_->GetGlassPtr()->SetVelocity(Vector3{});
                //パスされた時加算する
                glassManager_->AddShakeValue(0.15f);
                uiManager_->SetShakeProgress(glassManager_->GetShakeValue());
                JudgeAndSetCameraAndLoad();
                break;
            }
        }
    }

    //ここまで来たら重力処理をする
    glassManager_->GetGlassPtr()->AddTranslate(Vector3{ 0.0f, -0.06f, 0.0f });
}

void GameScenePhase::JudgeAndSetCameraAndLoad()
{   
    //グラスが地面ついて割れました。
    const float humanDegree = humanManager_->GetCurrentGlassUserDegree();

    gameCameraManager_->SetClosestThetaRadian(humanDegree);
    gameCameraManager_->ChangeCameraPhase(CameraPhase::CatchFollowing);
    //グラス位置の算出
    glassManager_->SetPosForTableAndHuman(table_->GetTranslate(), table_->GetRadius(), humanDegree, 1.28f + 0.06f);

    //破壊数の計算
    int32_t obstacleCount = obstacles_.size();
    uiManager_->GetBreakEvaluation()->SetMaxBreakCount(maxBreakableObstacleCount_);
    uiManager_->GetBreakEvaluation()->SetBreakCount(maxBreakableObstacleCount_ - obstacleCount);

    //ロードに入る　ここで最大破壊可能障害物の設定をする
    LoadObstacleData(Game::Math::Rand::RandInt(0, stageSum));
}

void GameScenePhase::PlayerControl()
{
    if (!canControll_) {
        return;
    }
    glassManager_->PlayerControl(gameCameraManager_.get(), obstacles_);

}

void GameScenePhase::UpdateBGM()
{
    GameBGMSystem::GetInstance().UpVolume(GameBGMSystem::GAME_BGM);
}

void GameScenePhase::ResetGame()
{    
    //Rキーを押したらリスタート
    if (!Game::IO::Key::IsJustPressed('R'))
    {
        return;
    }

    const float currentHumanDegree = humanManager_->GetCurrentGlassUserDegree();
    //ゲームカメラ管理
    gameCameraManager_->SetClosestThetaRadian(currentHumanDegree);
    gameCameraManager_->ChangeCameraPhase(CameraPhase::CatchFollowing);

    //初期化を呼んでみる
    glassManager_->Initialize();
    glassManager_->SetPosForTableAndHuman(table_->GetTranslate(), table_->GetRadius(), currentHumanDegree, 1.28f);

    uiManager_->Initialize();
}

void GameScenePhase::DrawMainScreen(const int32_t renderTexture)
{
    //バーの描画
    bar_->Draw(renderTexture);
    //人間の描画
    humanManager_->Draw(renderTexture);
    //テーブルの描画
    table_->Draw(renderTexture);

    if (gameCameraManager_->GetCameraPhase() == CameraPhase::ShotAngleSetup)
    {
        //グラス総括管理の予測線描画
        glassManager_->DrawPrediction(renderTexture);
    }
    // 障害物の描画
    for (auto& obstacle : obstacles_)
    {
        obstacle->Draw(renderTexture);
    }

    glassParticle_->Draw(renderTexture);

    //グラス総括管理
    glassManager_->Draw(renderTexture);

    //コライダーデバック描画
    if (isDebugDraw_) collisionManager_->DebugDraw(renderTexture);
}

bool GameScenePhase::LoadObstacleData(int32_t stage)
{

    // ★追加: ステージ切り替え時は破棄予約インデックスをリセットする
    deleteIndex = -1;

    int32_t count = 0;
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string key = "/Stage" + std::to_string(stage) + "/Count";
    bool success = JsonManager::Load(path, key, count);
    if (!success) return false;

    maxBreakableObstacleCount_ = count;

    obstacles_.clear();
    obstacles_.resize(count);

    for (int32_t i = 0; i < count; i++)
    {
        obstacles_[i] = std::make_unique<TableObject>();
        obstacles_[i]->Initialize();

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
        std::string typeStr;
        JsonManager::Load(path, key, typeStr);

        auto type = magic_enum::enum_cast<GlassType>(typeStr);
        if (type.has_value()) {
            obstacles_[i]->SetGlassTypeAndLoadModels(type.value());
        } else {
            // エラーログやデフォルト値の設定
            //日本酒
            obstacles_[i]->SetGlassTypeAndLoadModels(GlassType::JapaneseSake);
        }

  /*      obstacles_[i]->SetGlassTypeAndLoadModels(magic_enum::enum_cast<GlassType>(typeStr).value());*/

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
        Vector3 translate;
        JsonManager::Load(path, key, translate);
        obstacles_[i]->SetTranslate(translate);


        obstacles_[i]->SetLightData(&gameLight_->GetLightData());
    }

    //人間管理　マーカーも一緒！
    success = humanManager_->Load(path, stage, table_->GetTranslate(), table_->GetRadius());
    if (!success) return false;

    int32_t y = 0;
    while (true)
    {
        key = "/Stage" + std::to_string(y) + "/Count";
        int32_t t;
        bool success = JsonManager::Load(path, key, t);
        if (!success) break;
        y++;
    }
    stageSum = y;


    // ★追加: ロードによって旧障害物が破棄されたため、コライダーリストを即座にクリアする
    if (collisionManager_) {
        collisionManager_->ClearColliders();
    }

    return true;
}

void GameScenePhase::SaveObstacleData(int32_t stage)
{
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string key = "/Stage" + std::to_string(stage) + "/Count";

    int32_t count = static_cast<int32_t>(obstacles_.size());

    JsonManager::AddParam(path, key, count);
    for (int32_t i = 0; i < count; i++)
    {
        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
        JsonManager::AddParam(path, key, magic_enum::enum_name(obstacles_[i]->GetGlassType()));

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
        JsonManager::AddParam(path, key, obstacles_[i]->GetTranslate());
    }

    //セーブデータをセットする
    humanManager_->Save(path, stage);

    JsonManager::Save(path);
}
