#include "SikouteiDevelopPhase.h"
#include <GameObject/Glass/Glass.h>
#include <GameObject/TableObject/TableObject.h>
#include <GameObject/Table/Table.h>
#include <GameObject/CocktailWater/CocktailWater.h>
#include <GameObject/HumanModel/HumanModel.h>
#include <GameObject/Bartender/Bartender.h>
#include <GameObject/Customer/Customer.h>
#include <System/CollisionManager/CollisionManager.h>
#include <Utilities/Json/JsonManager.h>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>
#include<GameObject/PredictionObj/PredictionObj.h>
#include<GameObject/SimpleObstaclePlacementFlow/SimpleObstaclePlacementFlow.h>
#include<GameObject/UI/UIManager/UIManager.h>

namespace
{
    /// <summary>
    /// 大きい円の内壁と小さい円が接触しているかを判定する
    /// </summary>
    /// <param name="bigCenter">でかい円の中心</param>
    /// <param name="bigRadius">でかい円の半径</param>
    /// <param name="smallCenter">小さい円の中心</param>
    /// <param name="smallRadius">小さい円の半径</param>
    /// <returns></returns>
    bool IsTouchingInnerEdge(
        const Vector2& bigCenter,
        float bigRadius,
        const Vector2& smallCenter,
        float smallRadius)
    {
        float dist = (smallCenter - bigCenter).Length();
        return (dist + smallRadius) >= bigRadius;
    }
    
    /// <summary>
	/// でかい円から見て小さい円がどの方向にあるかを返す
    /// </summary>
    /// <param name="bigCenter">でかい円の中心</param>
    /// <param name="smallCenter">小さい円の中心</param>
    /// <returns> 0〜360</returns>
    float GetContactAngleDeg(const Vector2& bigCenter, const Vector2& smallCenter)
    {
        Vector2 dir = smallCenter - bigCenter;
        if (dir.LengthSq() < 0.0001f)
        {
            return 0.0f;
        }

        float angleRad = std::atan2(dir.y, dir.x);
        float angleDeg = angleRad * (180.0f / std::numbers::pi_v<float>);
        if (angleDeg < 0.0f) angleDeg += 360.0f;
        return angleDeg; // 0〜360
    }

    // angleDegがfromDegからtoDegの範囲に入っているか
    bool IsInAngleRange(float angleDeg, float fromDeg, float toDeg)
    {
        auto Normalize = [](float a)
            {
                a = fmod(a, 360.0f);
                if (a < 0.0f) a += 360.0f;
                return a;
            };

        float a = Normalize(angleDeg);
        float f = Normalize(fromDeg);
        float t = Normalize(toDeg);

        if (f <= t)
        {
            // 通常の範囲（例：30°～120°）
            return a >= f && a < t;
        }
        else
        {
            // 360°を跨ぐ範囲（例：350°～20°）
            return a >= f || a < t;
        }
    }


    Vector3 GetPositionOnCircle(const Vector3& center, float radius, float angleDeg)
    {
        float angleRad = angleDeg * (std::numbers::pi_v<float> / 180.0f);
        return Vector3(
            center.x + radius * std::cos(angleRad),
            center.y,
            center.z + radius * std::sin(angleRad)
        );
    }

    float NormalizeAngle(float a)
    {
        a = std::fmod(a, 360.0f);
        if (a < 0) a += 360.0f;
        return a;
    }

    float AngleDiff(float a, float b)
    {
        float diff = NormalizeAngle(a - b);
        if (diff > 180.0f) diff -= 360.0f;
        return diff;
    }

    float ClosestAngle(float target, float base)
    {
        float diff = AngleDiff(target, base);
        return target - diff;
    }

    // 現在のtheta(ラジアン)から見て、targetDeg(度)と等価な角度のうち最短距離になるものをラジアンで返す
    float ClosestThetaRadian(float currentThetaRad, float targetDeg)
    {
        float currentDeg = Game::Math::Converter::RadianToDegree(currentThetaRad);
        return Game::Math::Converter::DegreeToRadian(ClosestAngle(currentDeg, targetDeg));
    }
}



SikouteiDevelopPhase::SikouteiDevelopPhase()
{
    // レンダーターゲット
    renderTargetID_ = Game::Asset::RenderTexture::CreateRenderTexture(Game::Window::GetWidth(), Game::Window::GetHeight(), "SikouteiDevelop");

    // サウンド
	s_GameScene_ = Game::Asset::Audio::Load("assets/application/audio/BGM/GameScene.mp3");

    // カメラ
    c_main_ = Game::Camera::AddCamera("SikouteiDevelopPhase");

    //コリジョン管理
    collisionManager_->Load();
    collisionManager_ = std::make_unique<CollisionManager>();


    // オブジェクト実体生成
	cocktailWater_ = std::make_unique<CocktailWater>();
    table_ = std::make_unique<Table>();
    table_->SetLightData(&lightData_);
    glass_ = std::make_unique<Glass>();
    glass_->SetLightData(&lightData_);
    for (int32_t i = 0; i < Constexprs::kMaxObstacleCount; i++)
    {
        obstacles_[i] = std::make_unique<TableObject>();
        obstacles_[i]->Initialize();
        obstacles_[i]->SetLightData(&lightData_);
    }

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
        
        if (i == 2) {
            //インデックスの一番目がカスタマー
            human_[i] = std::make_unique<Customer>();
        } else {
            //バーテンダー
            human_[i] = std::make_unique<Bartender>();
        }

        human_[i]->Load();
        human_[i]->SetLightData(&lightData_);
	}

	bar_ = std::make_unique<RenderObject>();
    bar_->psoConfig_.vs = "assets/shaders/PunctualLight/PunctualLight.VS.hlsl";
    bar_->psoConfig_.ps = "assets/shaders/PunctualLight/PunctualLight.PS.hlsl";
    bar_->modelID_ = Game::Asset::Model::Load("assets/application/model/Bar/Bar.obj");
    bar_->SetupFromShaders();
    barTextureID_ = Game::Asset::Texture::Load("assets/application/model/Bar/Bar.png");

    prediction_ = std::make_unique<PredictionObj>();
	prediction_->SetCollisionManager(collisionManager_.get());
    prediction_->SetObstacleArray(obstacles_);
    prediction_->SetLightData(&lightData_);

    Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(45.0f), 0.0f, EaseType::OUT_BACK, c_main_);

    simpleObstaclePlacementFlow_ = std::make_unique<SimpleObstaclePlacementFlow>();

    barTransforms_.scale = Vector3{ 0.1f, 0.1f, 0.1f };
    barTransforms_.translate = Vector3{ 0.0f, 0.6f, 9.0f };

    uiManager_ = std::make_unique<UIManager>();

}

SikouteiDevelopPhase::~SikouteiDevelopPhase()
{}

void SikouteiDevelopPhase::Initialize()
{


    isShot_ = false;
	nextPhase_ = Phase::Phase_None;
    context_->renderTargetIDs[static_cast<size_t>(Phase::Phase_SikouteiDevelop)] = renderTargetID_;

	// オブジェクト初期化
    table_->Initialize();
    glass_->Initialize();
	cocktailWater_->Initialize();
    //予測オブジェクト
    prediction_->Initialize();

    for (int32_t i = 0; i < 3; i++)
    {
        human_[i]->Initialize();
    }

    EulerTransforms spawnTransform;
    spawnTransform.translate = Vector3(0.0f, 10.0f, -5.0f); // 空中の発射場所
    spawnTransform.rotate = Vector3(0.0f, 0.0f, 0.0f);
    spawnTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
    simpleObstaclePlacementFlow_->SetSpawnPoint(spawnTransform);
    
    LoadObstacleData(0);

	LoadLightData();

    volume = 0.0f;
    s_GameScene_PlayIDs_.push_back(Game::Audio::PlayAudio(s_GameScene_, true, volume));


    cameraSpherical_.theta = Game::Math::Converter::DegreeToRadian(humanRotateDegree[0]);
    Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.2f, EaseType::OUT_BACK, c_main_);


    cameraSpherical_.theta = ClosestThetaRadian(cameraSpherical_.theta, humanRotateDegree[currentGlassUserIndex_]);
    ChangeCameraPhase(CameraPhase::CatchFollowing);
    Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotateDegree[currentGlassUserIndex_]);
    glassPos.y = 1.28f;
    glass_->SetTranslate(glassPos);
    glass_->SetVelocity(Vector3{});


    //UI管理 ゲームタイマー
    uiManager_->Initialize();

}

void SikouteiDevelopPhase::Update()
{


    if (uiManager_->GetTimer() <= 0.0f) {
        //フェーズ
        nextPhase_ = Phase::Phase_Result;
    }

    Game::Camera::Update(c_main_);

    if (Game::IO::Key::IsJustPressed('R'))
    {
        cameraSpherical_.theta = ClosestThetaRadian(cameraSpherical_.theta, humanRotateDegree[currentGlassUserIndex_]);
        ChangeCameraPhase(CameraPhase::CatchFollowing);
        Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotateDegree[currentGlassUserIndex_]);
        glassPos.y = 1.28f;
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
    if (IsTouchingInnerEdge(tablePos2D, table_->GetRadius(), glassPos2D, glass_->GetRadius()))
    {
        float angle = GetContactAngleDeg(tablePos2D, glassPos2D);
        for (int i = 0; i < 3; i++)
        {
            if (IsInAngleRange(angle, markerAngles_[i * 2], markerAngles_[i * 2 + 1]))
            {
                currentGlassUserIndex_ = i;
          
                cameraSpherical_.theta = ClosestThetaRadian(cameraSpherical_.theta, humanRotateDegree[currentGlassUserIndex_]);

				ChangeCameraPhase(CameraPhase::CatchFollowing);
                Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotateDegree[currentGlassUserIndex_]);
                glassPos.y = 1.28f + 0.06f;
                glass_->SetTranslate(glassPos);
                glass_->SetVelocity(Vector3{});
                break;
            }
        }


        glass_->AddTranslate(Vector3{ 0.0f, -0.06f, 0.0f });
    }

    if (deleteIndex >= 0)
    {
        obstacles_[deleteIndex] = std::move(obstacles_[obstacleCount - 1]);
        obstacleCount--;
        deleteIndex = -1;
    }

  
    simpleObstaclePlacementFlow_->Update();
    for (int i = 0; i < obstacleCount; ++i) {
        obstacles_[i]->SetTranslate(simpleObstaclePlacementFlow_->GetPieces(i).transform.translate);
    }

    // オブジェクト更新
    glass_->Update(c_main_);
    cocktailWater_->SetTranslate(glass_->GetTranslate() + Vector3{ 0.0f,-0.09f,0.0f });
    cocktailWater_->Update(c_main_);
    table_->Update(c_main_);
    for (int32_t i = 0; i < obstacleCount; i++)
    {
        obstacles_[i]->Update(c_main_);
    }
    for (int32_t i = 0; i < 3; i++)
    {
        human_[i]->SetIsShot(isShot_);
        human_[i]->Update(c_main_);
    }

    //人間に発射フラグを渡したら発射を毎フレーム偽にする
    isShot_ = false;

    //コライダー更新
    if (isDebugDraw_) collisionManager_->DebugUpdate(c_main_);
    CheckColliders();

    // ショット
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
            }
            else
            {
                velocity_ = Vector2(0.0f, 0.0f);
            }

            Vector3 velocity = { velocity_.x, 0.0f, velocity_.y };
            prediction_->SetObstacleCount(obstacleCount);
            prediction_->SetVelocity(velocity);
            prediction_->SetTranslate(glass_->GetTranslate());
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
            }
            else
            {
                ChangeCameraPhase(CameraPhase::Free);
            }
        }

        prediction_->Update(c_main_);
    }


    UpdateCameraPhase();
    Vector3 velocity = { velocity_.x, 0.0f, velocity_.y };
    prediction_->SetVelocity(velocity);
    prediction_->SetTranslate(glass_->GetTranslate());
    prediction_->Update(c_main_);

    //UI管理
    uiManager_->Update();

}

void SikouteiDevelopPhase::Draw()
{
    const Matrix4x4 viewPro = Game::Camera::Getter::GetViewProjectionMatrix(c_main_);
    const int32_t white1x1 = Game::Asset::Texture::Load("assets/engine/texture/white1x1.png");
	const Vector3 cameraPos = Game::Camera::Getter::GetWorldPosition(c_main_);

    for (int32_t i = 0; i < 3; i++)
    {
        human_[i]->Draw(renderTargetID_);
    }
    for (int32_t i = 0; i < 6; i++)
    {
        Matrix4x4 world = markerTransforms_[i].GetWorldMatrix();
        Matrix4x4 wvp = world * viewPro;
        Vector4 color = Vector4{ 1.0f, 0.0f, 0.0f, 1.0f };

        markers_[i]->SetCBufferData(0, ShaderType::VertexShader, &wvp);
        markers_[i]->SetCBufferData(1, ShaderType::VertexShader, &world);
        markers_[i]->SetCBufferData(0, ShaderType::PixelShader, &color);
        markers_[i]->SetCBufferData(1, ShaderType::PixelShader, &white1x1);
        markers_[i]->Draw(renderTargetID_);
    }
    {
        Matrix4x4 world = barTransforms_.GetWorldMatrix();
        Matrix4x4 wvp = world * viewPro;
        Vector4 color = Vector4{ 1.0f, 0.0f, 0.0f, 1.0f };

        bar_->SetCBufferData(0, ShaderType::VertexShader, &wvp);
        bar_->SetCBufferData(1, ShaderType::VertexShader, &world);
        bar_->SetCBufferData(0, ShaderType::PixelShader, &cameraPos);
        bar_->SetCBufferData(1, ShaderType::PixelShader, &lightData_);
        bar_->SetCBufferData(2, ShaderType::PixelShader, &barMaterial_);
        bar_->SetCBufferData(3, ShaderType::PixelShader, &barTextureID_);
		bar_->Draw(renderTargetID_);
    }

    //テーブルの描画
    table_->Draw(renderTargetID_);

    if (cameraPhase_ == CameraPhase::ShotAngleSetup)
    {
        prediction_->Draw(renderTargetID_);
    }

	// 障害物の描画
    for (int32_t i = 0; i < obstacleCount; i++)
	{
		obstacles_[i]->Draw(renderTargetID_);
	}
	cocktailWater_->Draw(renderTargetID_);
    //グラスは半透明なので後に描画する
    glass_->Draw(renderTargetID_);

    //コライダーデバック描画
    if (isDebugDraw_) collisionManager_->DebugDraw();
    //UIなので一番最後に描画する
    uiManager_->Draw();
}

void SikouteiDevelopPhase::DrawImGui()
{
    glass_->DrawImGui();
    cocktailWater_->DrawImGui();
    obstacles_[0]->DrawImGui();
    table_->DrawImGui();
    prediction_->DrawImGui();
    //collisionManager_->DebugImGui();
    uiManager_->DebugImGui();
    human_[2]->DrawImGui();
    ImGui::Begin("glass");

    Vector3 glassVel = glass_->GetVelocity();
    Vector3 glassVel2dNormalized = Vector3(glassVel.x, 0.0f, glassVel.z).Normalized();
    Vector3 yawPttch = Game::Math::YawPitchFromDirection(glassVel2dNormalized);
	ImGui::Text("glass yawPitch: %f, %f, %f", yawPttch.x, yawPttch.y, yawPttch.z);
    ImGui::End();

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
            obstacles_[obstacleCount]->Initialize();
            obstacles_[obstacleCount]->SetGlassTypeAndLoadModels(glassType);
            Vector3 pos = { position.x, 1.28f, position.y };
            obstacles_[obstacleCount]->SetTranslate(pos);   //ここで読み込む
            simpleObstaclePlacementFlow_->ReadObstaclePlacement(obstacles_[obstacleCount]->GetTransform());
            obstacleCount++;
        }

        ImGui::TreePop();
    }

    // 障害物リスト
    if (ImGui::TreeNode("List"))
    {
        for (size_t i = 0; i < obstacleCount; ++i)
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
                    simpleObstaclePlacementFlow_->SetPieceLocalPosition(static_cast<int32_t>(i), newPos);
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

    // 人間の位置
    if (ImGui::TreeNode("Human"))
    {
        bool edit = false;
        if (ImGui::DragFloat3("HumanRotate", humanRotateDegree, 1.0f, -360.0f, 720.0f)) edit = true;
        if (ImGui::DragFloat("HumanScale", &humansize_, 1.0f, 0.0f, 120.0f)) edit = true;

        if (edit)
        {
            Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotateDegree[0]);
            glassPos.y = 1.28f;
            glass_->SetTranslate(glassPos);

            for (int32_t i = 0; i < 3; i++)
            {
				Vector3 humanPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 1.2f, humanRotateDegree[i]);
                humanPos.y = 1.28f;
				human_[i]->SetTranslate(humanPos);
            }

            for (int32_t i = 0; i < 6; i++)
            {
                float hugou = i % 2 == 0 ? -1.0f : 1.0f;
                float angle = humanRotateDegree[(i / 2)] + (hugou * humansize_ * 0.5f);
                markerAngles_[i] = angle;
            }
            for (int32_t i = 0; i < 6; i++)
            {
                markerTransforms_[i].translate = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.8f, markerAngles_[i]);
                markerTransforms_[i].translate.y = 1.28f;
                markerTransforms_[i].scale = Vector3{ 0.1f,0.1f,0.1f };
            }
        }

        ImGui::TreePop();
    }

    // ライト
    if (ImGui::TreeNode("LightData"))
    {
		if (ImGui::Button("Load", ImVec2(40, 20))) LoadLightData();
		ImGui::SameLine();
		if (ImGui::Button("Save", ImVec2(40, 20))) SaveLightData();

        ImGui::DragInt("LightCount", &lightData_.LightCount, 1, 0, 4);
            ImGui::ColorEdit3("ambientColor", &lightData_.ambientColor.x);
        for (int i = 0; i < lightData_.LightCount; ++i)
        {

            std::string lightNodeName = "Light" + std::to_string(i);
            if (ImGui::TreeNode(lightNodeName.c_str()))
            {
                ImGui::SeparatorText("Common");
                ImGui::ColorEdit3("color", &lightData_.lights[i].color.x);
                ImGui::DragFloat("intensity", &lightData_.lights[i].intensity, 0.01f);

                ImGui::SeparatorText("type");
                ImGui::DragInt("type", &lightData_.lights[i].type, 1, 0, 2);

                ImGui::SeparatorText("Directional");
                ImGui::DragFloat3("direction", &lightData_.lights[i].direction.x, 0.01f);

                ImGui::SeparatorText("Spot");
                ImGui::DragFloat("spotCos", &lightData_.lights[i].spotCos, 0.01f);

                ImGui::SeparatorText("Point / Spot");
                ImGui::DragFloat3("position", &lightData_.lights[i].position.x, 0.01f);
                ImGui::DragFloat("range", &lightData_.lights[i].range, 0.01f);

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    // 人間の位置
    if (ImGui::TreeNode("Bar"))
    {
        ImGui::DragFloat3("Scale", &barTransforms_.scale.x, 0.01f);
        ImGui::DragFloat3("Translate", &barTransforms_.translate.x, 1.0f);

        ImGui::TreePop();
    }

    simpleObstaclePlacementFlow_->DrawImGui();

    ImGui::End();
}

void SikouteiDevelopPhase::CheckColliders()
{
    //コライダーリストを毎フレーム削除してみる？
    collisionManager_->ClearColliders();

    //コライダーを追加する
    for (auto& collider : glass_->GetColliders())
    {
        collisionManager_->AddCollider(collider.get());
    }

    for (int32_t i = 0; i < obstacleCount; i++)
    {
        for (auto& collider : obstacles_[i]->GetColliders())
        {
            collisionManager_->AddCollider(collider.get());
        }
    }

    for (auto& collider : table_->GetColliders())
    {
        collisionManager_->AddCollider(collider.get());
    }

    //for (auto& prediction : prediction_->GetColliders())
    //{
    //    collisionManager_->AddCollider(prediction.get());
    //}

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}


bool SikouteiDevelopPhase::LoadObstacleData(int32_t stage)
{
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string key = "/Stage" + std::to_string(stage) + "/Count";
	bool success = JsonManager::Load(path, key, obstacleCount);
	if (!success) return false;

    simpleObstaclePlacementFlow_->Initialize();

    for (int32_t i = 0; i < obstacleCount; i++)
    {
        obstacles_[i]->Initialize();

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
		std::string typeStr;
        JsonManager::Load(path, key, typeStr);
		obstacles_[i]->SetGlassTypeAndLoadModels(magic_enum::enum_cast<GlassType>(typeStr).value());

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
        Vector3 translate;
        JsonManager::Load(path, key, translate);
	/*	obstacles_[i]->SetTranslate(translate);*/

        //後で回転対応させる
        EulerTransforms tempTransform = { 
            .scale = {1.0f,1.0f,1.0f},
             .rotate = {0.0f,0.0f,0.0f},
            .translate = translate
        };

        //ここで読み込む
        simpleObstaclePlacementFlow_->ReadObstaclePlacement(tempTransform);
    }

    key = "/Stage" + std::to_string(stage) + "/Human/RotateDeg";
    Vector3 humanRotateDeg;
    JsonManager::Load(path, key, humanRotateDeg);
	humanRotateDegree[0] = humanRotateDeg.x;
	humanRotateDegree[1] = humanRotateDeg.y;
	humanRotateDegree[2] = humanRotateDeg.z;
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";
    JsonManager::Load(path, key, humansize_);

    const float pi = std::numbers::pi_v<float>*2.0f/3.0f;
    for (int32_t i = 0; i < 3; i++)
    {
        Vector3 humanPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.8f, humanRotateDegree[i]);
        humanPos.y = 0.0f;
        human_[i]->SetTranslate(humanPos);
        human_[i]->SetGlassPos(&glass_->GetTranslatePointer());
        human_[i]->SetRotateY(-pi*i);
    }

    for (int32_t i = 0; i < 6; i++)
    {
        float hugou = i % 2 == 0 ? -1.0f : 1.0f;
        float angle = humanRotateDegree[(i / 2)] + (hugou * humansize_ * 0.5f);
        markerAngles_[i] = angle;
    }
    for (int32_t i = 0; i < 6; i++)
    {
        markerTransforms_[i].translate = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.8f, markerAngles_[i]);
        markerTransforms_[i].translate.y = 1.28f;
        markerTransforms_[i].scale = Vector3{ 0.1f,0.1f,0.1f };
    }


    simpleObstaclePlacementFlow_->HideAllPieces();
    simpleObstaclePlacementFlow_->StartPlacement();

	return true;
}

void SikouteiDevelopPhase::SaveObstacleData(int32_t stage)
{
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string key = "/Stage" + std::to_string(stage) + "/Count";
	JsonManager::AddParam(path, key, obstacleCount);
	for (int32_t i = 0; i < obstacleCount; i++)
	{
		key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
		JsonManager::AddParam(path, key, magic_enum::enum_name(obstacles_[i]->GetGlassType()));

		key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
		JsonManager::AddParam(path, key, obstacles_[i]->GetTranslate());
	}

	key = "/Stage" + std::to_string(stage) + "/Human/RotateDeg";
	Vector3 humanRotateDeg = { humanRotateDegree[0], humanRotateDegree[1], humanRotateDegree[2] };
    JsonManager::AddParam(path, key, humanRotateDeg);
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";
    JsonManager::AddParam(path, key, humansize_);

	JsonManager::Save(path);
}


bool SikouteiDevelopPhase::LoadLightData()
{
    std::string path = "assets/application/json/StageData/Lights.json";
    std::string key = "/Count";
    bool success = JsonManager::Load(path, key, lightData_.LightCount);
    if (!success) return false;

	key = "/ambientColor";
    success = JsonManager::Load(path, key, lightData_.ambientColor);
	if (!success) return false;

    for (int32_t i = 0; i < lightData_.LightCount; i++)
    {
        key = "/Light" + std::to_string(i) + "/color";
        JsonManager::Load(path, key, lightData_.lights[i].color);
        key = "/Light" + std::to_string(i) + "/intensity";
        JsonManager::Load(path, key, lightData_.lights[i].intensity);
        key = "/Light" + std::to_string(i) + "/direction";
        JsonManager::Load(path, key, lightData_.lights[i].direction);
        key = "/Light" + std::to_string(i) + "/spotCos";
        JsonManager::Load(path, key, lightData_.lights[i].spotCos);
        key = "/Light" + std::to_string(i) + "/position";
        JsonManager::Load(path, key, lightData_.lights[i].position);
        key = "/Light" + std::to_string(i) + "/range";
        JsonManager::Load(path, key, lightData_.lights[i].range);
        key = "/Light" + std::to_string(i) + "/position";
        JsonManager::Load(path, key, lightData_.lights[i].position);
        key = "/Light" + std::to_string(i) + "/type";
        JsonManager::Load(path, key, lightData_.lights[i].type);
    }

    return true;
}

void SikouteiDevelopPhase::SaveLightData()
{
    std::string path = "assets/application/json/StageData/Lights.json";
    std::string key = "/Count";
	JsonManager::AddParam(path, key, lightData_.LightCount);
	key = "/ambientColor";
	JsonManager::AddParam(path, key, lightData_.ambientColor);

	for (int32_t i = 0; i < lightData_.LightCount; i++)
	{
		key = "/Light" + std::to_string(i) + "/color";
		JsonManager::AddParam(path, key, lightData_.lights[i].color);
		key = "/Light" + std::to_string(i) + "/intensity";
		JsonManager::AddParam(path, key, lightData_.lights[i].intensity);
		key = "/Light" + std::to_string(i) + "/direction";
		JsonManager::AddParam(path, key, lightData_.lights[i].direction);
		key = "/Light" + std::to_string(i) + "/spotCos";
		JsonManager::AddParam(path, key, lightData_.lights[i].spotCos);
		key = "/Light" + std::to_string(i) + "/position";
		JsonManager::AddParam(path, key, lightData_.lights[i].position);
		key = "/Light" + std::to_string(i) + "/range";
		JsonManager::AddParam(path, key, lightData_.lights[i].range);
		key = "/Light" + std::to_string(i) + "/type";
		JsonManager::AddParam(path, key, lightData_.lights[i].type);
	}
	JsonManager::Save(path);
}



void SikouteiDevelopPhase::ChangeCameraPhase(CameraPhase phase)
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

		cameraPhaseCounter_.SetTargetTime(0.7f);

        break;
    }
    default:
        break;
    }
    cameraPhase_ = phase;
}

void SikouteiDevelopPhase::UpdateCameraPhase()
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

        cameraSpherical_.theta = ClosestThetaRadian(cameraSpherical_.theta, targetDeg);

		// thetaをグラス後方で固定
        Game::Camera::Setter::SetThetaTarget(cameraSpherical_.theta, 0.5f, EaseType::OUT_BACK, c_main_);

		// centerをグラスで固定
        Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

        break;
    }
    case CameraPhase::CatchFollowing:
    {
        if (cameraPhaseCounter_.CountUp())
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
