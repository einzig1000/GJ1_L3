#include "GameScenePhase.h"
#include <GameObject/Glass/Glass.h>
#include <GameObject/TableObject/TableObject.h>
#include <GameObject/Table/Table.h>
#include <GameObject/CocktailWater/CocktailWater.h>
//人間管理
#include<GameObject/HumanManager/HumanManager.h>
#include<GameObject/Bar/Bar.h>

#include <GameObject/PredictionObj/PredictionObj.h>

#include <GameObject/UI/UIManager/UIManager.h>
#include <GameObject/UI/BreakEvaluation/BreakEvaluation.h>
//ゲームライト
#include<GameObject/GameLight/GameLight.h>

#include <System/CollisionManager/CollisionManager.h>

#include <Utilities/Json/JsonManager.h>

#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
#include<System/GameFunction/GameFunction.h>

GameScenePhase::GameScenePhase()
{
    uiManager_ = std::make_unique<UIManager>();

    // レンダーターゲット
    rt_3D_ = Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(), Game::Window::GetHeight(),
        "gameScene_3D", Vector4{ 0.0f,0.0f,0.0f,0.0f });

    renderTargetID_ = Game::Asset::RenderTexture::CreateRenderTexture(
        Game::Window::GetWidth(), Game::Window::GetHeight(), "gameScene", Vector4{ 0.0f,0.0f,0.0f,0.0f });

    drawForMain_[0] = std::make_unique<RenderObject>();
    drawForMain_[0]->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
    drawForMain_[0]->psoConfig_.ps = "assets/shaders/FullScreen/CopyImage.PS.hlsl";
    drawForMain_[0]->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
    drawForMain_[0]->SetupFromShaders();
    drawForMain_[0]->SetCBufferData(0, ShaderType::PixelShader, &rt_3D_);

    drawForMain_[1] = std::make_unique<RenderObject>();
    drawForMain_[1]->psoConfig_.vs = "assets/shaders/FullScreen/FullScreen.VS.hlsl";
    drawForMain_[1]->psoConfig_.ps = "assets/shaders/FullScreen/CopyImage.PS.hlsl";
    drawForMain_[1]->modelID_ = Game::Asset::Model::Load("assets/engine/model/plane/plane.obj");
    drawForMain_[1]->SetupFromShaders();
    int32_t rtID = uiManager_->GetRenderTextureID();
    drawForMain_[1]->SetCBufferData(0, ShaderType::PixelShader, &rtID);

    // サウンド
    s_GameScene_ = Game::Asset::Audio::Load("assets/application/audio/BGM/GameScene.mp3");

    // カメラ
    c_main_ = Game::Camera::AddCamera("SikouteiDevelopPhase");

    //コリジョン管理
    collisionManager_->Load();
    collisionManager_ = std::make_unique<CollisionManager>();

    //ゲームライトの実体生成 ここで管理する
    gameLight_ = std::make_unique<GameLight>();

    // オブジェクト実体生成
    cocktailWater_ = std::make_unique<CocktailWater>();
    table_ = std::make_unique<Table>();
    table_->SetLightData(&gameLight_->GetLightData());
    glass_ = std::make_unique<Glass>();

    glass_->SetLightData(&gameLight_->GetLightData());

    //人間管理
    humanManager_ = std::make_unique<HumanManager>();
    humanManager_->SetLightData(&gameLight_->GetLightData());
    humanManager_->SetIsShotPtr(&isShot_);

    //予測線
    prediction_ = std::make_unique<PredictionObj>();
    prediction_->SetCollisionManager(collisionManager_.get());
    prediction_->SetLightData(&gameLight_->GetLightData());

    //配置用に使うつもりの残骸　もしかしたら後で参考にするかも
    //simpleObstaclePlacementFlow_ = std::make_unique<SimpleObstaclePlacementFlow>();

    //バー
    bar_ = std::make_unique<Bar>();
    bar_->SetLightData(&gameLight_->GetLightData());
}

GameScenePhase::~GameScenePhase() {}

void GameScenePhase::Initialize()
{
    isShot_ = false;
    nextPhase_ = Phase::Phase_None;
    context_->renderTargetIDs[static_cast<size_t>(Phase::Phase_GameScene)] = renderTargetID_;

    // オブジェクト初期化
    table_->Initialize();
    glass_->Initialize();
    cocktailWater_->Initialize();
    //予測オブジェクト
    prediction_->Initialize();

    //人間管理
    humanManager_->Initialize();

    //バー初期化（今は中身なし）
    bar_->Initialize();

    volume = 0.0f;
    s_GameScene_PlayIDs_.push_back(Game::Audio::PlayAudio(s_GameScene_, true, volume));

    //ゲームライトのロード
    gameLight_->Load();

    LoadObstacleData(0);

    cameraSpherical_.phi = 20.0f;
    Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.2f, EaseType::LINEAR, c_main_);
    cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, humanManager_->GetCurrentGlaassUserDegree());
    Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.2f, EaseType::OUT_BACK, c_main_);

    ChangeCameraPhase(CameraPhase::CatchFollowing);
    Vector3 glassPos = GameFunction::GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanManager_->GetCurrentGlaassUserDegree());
    glassPos.y = 1.28f;
    glass_->SetTranslate(glassPos);
    glass_->SetVelocity(Vector3{});
    
    //UI管理 ゲームタイマー
    uiManager_->Initialize();
}

void GameScenePhase::Update()
{
#ifdef _RELEASE
    //リリース版ならシーン切り替えする
    if (uiManager_->GetTimer() <= 0.0f) {
        //フェーズ
        nextPhase_ = Phase::Phase_Result;
    }
#endif

    //カメラの更新
    Game::Camera::Update(c_main_);
    //Rキーを押したらリスタート
    if (Game::IO::Key::IsJustPressed('R'))
    {

        const float currentHumanDegree = humanManager_->GetCurrentGlaassUserDegree();
        cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, currentHumanDegree);
        
        ChangeCameraPhase(CameraPhase::CatchFollowing);
        Vector3 glassPos = GameFunction::GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, currentHumanDegree);
        glassPos.y = 1.28f;
        glass_->ResetBroken();
        glass_->SetTranslate(glassPos);
        glass_->SetVelocity(Vector3{});
        uiManager_->Initialize();
    }

    volume += Game::Time::GetScaledDeltaTimeMs() * 0.0001f;
    volume = std::clamp(volume, 0.0f, 1.0f);
    Game::Audio::SetAudioVolume(s_GameScene_PlayIDs_[0], volume);

    // テーブル外判定
    Vector2 tablePos2D = Vector2(table_->GetTranslate().x, table_->GetTranslate().z);
    Vector2 glassPos2D = Vector2(glass_->GetTranslate().x, glass_->GetTranslate().z);
  
    //テーブルの内円とグラスの外円が接触しているか
    if (GameFunction::IsTouchingInnerEdge(tablePos2D, table_->GetRadius(), glassPos2D, glass_->GetRadius()))
    {
        float angle = GameFunction::GetContactAngleDeg(tablePos2D, glassPos2D);
      
        for (int i = 0; i < 3; i++)
        {
            humanManager_->GetMarkerAngle(i * 2);

            //angleがfromDegからtoDegの範囲に入っているか
            if (GameFunction::IsInAngleRange(
                angle,
                humanManager_->GetMarkerAngle(i * 2),
                humanManager_->GetMarkerAngle(i * 2+1)
            
            ))
            {
          
                humanManager_->SetCurrentGlassUserIndex(i);
                const float humanDegree = humanManager_->GetCurrentGlaassUserDegree();
                cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, humanDegree);

                ChangeCameraPhase(CameraPhase::CatchFollowing);
                Vector3 glassPos = GameFunction::GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanDegree);
                glassPos.y = 1.28f + 0.06f;
                glass_->SetTranslate(glassPos);
                glass_->SetVelocity(Vector3{});
                uiManager_->GetBreakEvaluation()->SetMaxBreakCount(maxBreakableObstacleCount_);
                int32_t obstacleCount = obstacles_.size();
                uiManager_->GetBreakEvaluation()->SetBreakCount(maxBreakableObstacleCount_ - obstacleCount);
                uiManager_->AddScore(maxBreakableObstacleCount_ - obstacleCount);

                LoadObstacleData(Game::Math::Rand::RandInt(0, stageSum));
                break;
            }
        }


        glass_->AddTranslate(Vector3{ 0.0f, -0.06f, 0.0f });

        if (glass_->GetIsBroken())
        {
            const float humanDegree = humanManager_->GetCurrentGlaassUserDegree();
            cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, humanDegree);
            ChangeCameraPhase(CameraPhase::CatchFollowing);
            Vector3 glassPos = GameFunction::GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanDegree);
            glassPos.y = 1.28f;
            glass_->ResetBroken();
            glass_->SetTranslate(glassPos);
            glass_->SetVelocity(Vector3{});
        }
    }


    if (deleteIndex >= 0)
    {
        obstacles_[deleteIndex] = std::move(obstacles_.back());
        obstacles_.pop_back();
        deleteIndex = -1;
    }

    // オブジェクト更新
    glass_->Update(c_main_);
    table_->Update(c_main_);
    cocktailWater_->SetTranslate(glass_->GetTranslate() + Vector3{ 0.0f,-0.09f,0.0f });
    cocktailWater_->Update(c_main_);

    for (int32_t i = 0; i < obstacles_.size(); ++i)
    {
        obstacles_[i]->Update(c_main_);
        if (obstacles_[i]->IsBroken())
        {
            deleteIndex = i;
        }
    }

    //人間に発射フラグを渡したら発射を毎フレーム偽にする
    humanManager_->Update(c_main_);

    isShot_ = false;

    //コライダー更新
    if (isDebugDraw_) collisionManager_->DebugUpdate(c_main_);
    //コライダーの判定を開始する
    CheckColliders();
    //プレイヤー操作
    PlayerControl();

    //バーの更新
    bar_->Update(c_main_);

    //カメラのフェーズ
    UpdateCameraPhase();

    //UI管理
    uiManager_->Update();

}

void GameScenePhase::Draw()
{

    //バーの描画
    bar_->Draw(rt_3D_);
    //人間の描画
    humanManager_->Draw(rt_3D_);
 
    //テーブルの描画
    table_->Draw(rt_3D_);

    if (cameraPhase_ == CameraPhase::ShotAngleSetup)
    {
        prediction_->Draw(rt_3D_);
    }

    // 障害物の描画
    for (auto& obstacle : obstacles_)
    {
        obstacle->Draw(rt_3D_);
    }

    cocktailWater_->Draw(rt_3D_);

    //グラスは半透明なので後に描画する
    glass_->Draw(rt_3D_);

    //コライダーデバック描画
    if (isDebugDraw_) collisionManager_->DebugDraw();
    //UIなので一番最後に描画する
    uiManager_->Draw();

    drawForMain_[0]->Draw(renderTargetID_, { rt_3D_ });
    drawForMain_[1]->Draw(renderTargetID_, { uiManager_->GetRenderTextureID() });
}

void GameScenePhase::DrawImGui()
{
    glass_->DrawImGui();
    cocktailWater_->DrawImGui();
    //obstacles_[0]->DrawImGui();
    table_->DrawImGui();
    prediction_->DrawImGui();
    uiManager_->DrawImGui();

    humanManager_->DrawImGui(glass_.get(), table_.get());

    //バー
    bar_->DrawImGui();

    ImGui::Begin("Editor");

    ImGui::Checkbox("ableDrag", &ableDrag_);

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

    // Save & Load
    if (ImGui::Button("Load", ImVec2(50, 20)))
    {
        LoadObstacleData(currentStage_);
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
    for (auto& collider : glass_->GetColliders())
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

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}

void GameScenePhase::PlayerControl()
{    

    if (ableDrag_)
    {
        if (Game::IO::Mouse::IsJustPressed(0))
        {
            dragStartPos_ = Game::IO::Mouse::Get2DPosition();
            velocity_ = Vector2(0.0f, 0.0f);
            ChangeCameraPhase(CameraPhase::ShotAngleSetup);
            dragging_ = true;
        }
        if (dragging_ && Game::IO::Mouse::IsHeld(0))
        {
            Vector2 dragVector = Game::IO::Mouse::Get2DPosition() - dragStartPos_;
            float dragLengthY = dragStartPos_.y - Game::IO::Mouse::Get2DPosition().y;

            constexpr float kPowerScale = 0.05f; // 感度。要調整
            constexpr float kMaxSpeed = 5.0f;   // 上限。要調整

            if (dragLengthY > 0.0f)
            {
                Vector3 cameraDir = Game::Camera::Getter::GetCameraDirection(c_main_);
                cameraDir.y = 0.0f;
                cameraDir.Normalize();

                float power = std::clamp(dragLengthY * kPowerScale, 0.0f, kMaxSpeed);
                velocity_ = Vector2(cameraDir.x, cameraDir.z) * power;
            } else
            {
                velocity_ = Vector2(0.0f, 0.0f);
            }

            Vector3 velocity = { velocity_.x, 0.0f, velocity_.y };

            prediction_->SetVelocity(velocity);
            prediction_->SetTranslate(glass_->GetTranslate());
            //予測線の更新をする
            prediction_->Update(c_main_, obstacles_);
        }
        if (dragging_ && Game::IO::Mouse::IsJustReleased(0))
        {
            dragging_ = false;
            if (velocity_.LengthSq() > 0.5f)
            {
                glass_->SetVelocity(Vector3(velocity_.x, 0.0f, velocity_.y));
                cameraSpherical_.phi = 20.0f;

                ableDrag_ = false;
                prediction_->SetVelocity(Vector3{});
                //
                isShot_ = true;
                ChangeCameraPhase(CameraPhase::GlassFollowing);
            } else
            {
                ChangeCameraPhase(CameraPhase::Free);
            }
        }
    }

}

bool GameScenePhase::LoadObstacleData(int32_t stage)
{
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
        obstacles_[i]->SetGlassTypeAndLoadModels(magic_enum::enum_cast<GlassType>(typeStr).value());

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
        Vector3 translate;
        JsonManager::Load(path, key, translate);
        obstacles_[i]->SetTranslate(translate);


        obstacles_[i]->SetLightData(&gameLight_->GetLightData());
    }

    //人間管理　マーカーも一緒！
    success = humanManager_->Load(path, stage,table_->GetTranslate(),table_->GetRadius());
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
    humanManager_->Save(path,stage);

    JsonManager::Save(path);
}

void GameScenePhase::ChangeCameraPhase(CameraPhase phase)
{
    switch (phase)
    {
    case CameraPhase::Free:
    {
        Game::Camera::Setter::SetDistanceTarget(5.0f, 0.7f, EaseType::OUT_CIRC, c_main_);

        break;
    }
    case CameraPhase::ShotAngleSetup:
    {
        break;
    }
    case CameraPhase::GlassFollowing:
    {
        Game::Camera::Setter::SetDistanceTarget(1.0f, 0.2f, EaseType::LINEAR, c_main_);
        Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.2f, EaseType::LINEAR, c_main_);

        break;
    }
    case CameraPhase::CatchFollowing:
    {
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.7f, EaseType::OUT_CIRC, c_main_);

        Game::Camera::Setter::SetDistanceTarget(5.0f, 0.7f, EaseType::OUT_CIRC, c_main_);

        cameraPhaseCounter_.Initialize(0.7f);

        break;
    }
    default:
        break;
    }
    cameraPhase_ = phase;
}

void GameScenePhase::UpdateCameraPhase()
{
    switch (cameraPhase_)
    {
    case CameraPhase::Free:
    {
        // マウス移動量
        const Vector2 mouseDelta = Game::IO::Mouse::Get2DPositionDelta();
        constexpr float limit = 60.0f;


        // centerをグラスで固定
        Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

        break;
    }
    case CameraPhase::ShotAngleSetup:
    {
        // マウス移動量
        const Vector2 mouseDelta = Game::IO::Mouse::Get2DPositionDelta();
        constexpr float limit = 60.0f;

        // thetaをマウスで操作


        cameraSpherical_.theta -= mouseDelta.x * mouseInsensitivity_;

        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.0f, EaseType::OUT_BACK, c_main_);

        // centerをグラスで固定
        Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

        break;
    }
    case CameraPhase::GlassFollowing:
    {
        Vector3 glassVel = glass_->GetVelocity();
        Vector3 dir = Vector3(glassVel.x, 0.0f, glassVel.z).Normalized();
        //float theta = std::atan2(-dir.z, -dir.x); // 進行方向の逆(背後)
        float targetDeg = Game::Math::Converter::RadianToDegree(std::atan2(-dir.z, -dir.x));

        cameraSpherical_.theta = GameFunction::ClosestThetaRadian(cameraSpherical_.theta, targetDeg);

        // thetaをグラス後方で固定
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.5f, EaseType::OUT_BACK, c_main_);

        // centerをグラスで固定
        Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

        break;
    }
    case CameraPhase::CatchFollowing:
    {
        if (cameraPhaseCounter_.GetProgress() >= 1.0f)
        {
            ableDrag_ = true;
            ChangeCameraPhase(CameraPhase::Free);
        }

        // centerをグラスで固定
        Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

        break;
    }
    default:
        break;
    }
}
