#include "SikouteiDevelopPhase.h"
#include <GameObject/Glass/Glass.h>
#include <GameObject/TableObject/TableObject.h>
#include <GameObject/Table/Table.h>
#include <GameObject/CocktailWater/CocktailWater.h>
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
    /// <returns></returns>
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
}



SikouteiDevelopPhase::SikouteiDevelopPhase()
{
    // カメラ
    c_main_ = Game::Camera::AddCamera("SikouteiDevelopPhase");

    //コリジョン管理
    collisionManager_->Load();
    collisionManager_ = std::make_unique<CollisionManager>();


    // オブジェクト実体生成
	cocktailWater_ = std::make_unique<CocktailWater>();
    table_ = std::make_unique<Table>();
    glass_ = std::make_unique<Glass>();
    for (int32_t i = 0; i < Constexprs::kMaxObstacleCount; i++)
    {
        obstacles_[i] = std::make_unique<TableObject>();
        obstacles_[i]->Initialize();
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
		human_[i] = std::make_unique<RenderObject>();
		human_[i]->psoConfig_.vs = "assets/shaders/SimpleModel/SimpleModel.VS.hlsl";
		human_[i]->psoConfig_.ps = "assets/shaders/SimpleModel/SimpleModel.PS.hlsl";
		human_[i]->SetupFromShaders();
		human_[i]->modelID_ = Game::Asset::Model::Load("assets/engine/model/cube/cube.obj");
	}


    prediction_ = std::make_unique<PredictionObj>();


    simpleObstaclePlacementFlow_ = std::make_unique<SimpleObstaclePlacementFlow>();

    uiManager_ = std::make_unique<UIManager>();

}

SikouteiDevelopPhase::~SikouteiDevelopPhase()
{}

void SikouteiDevelopPhase::Initialize()
{
	// オブジェクト初期化
    table_->Initialize();
    glass_->Initialize();
	cocktailWater_->Initialize();
    //予測オブジェクト
    prediction_->Initialize();
    //UI管理
    uiManager_->Initialize();

    EulerTransforms spawnTransform;
    spawnTransform.translate = Vector3(0.0f, 10.0f, -5.0f); // 空中の発射場所
    spawnTransform.rotate = Vector3(0.0f, 0.0f, 0.0f);
    spawnTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
    simpleObstaclePlacementFlow_->SetSpawnPoint(spawnTransform);
    
    LoadObstacleData(0);

    simpleObstaclePlacementFlow_->HideAllPieces();
    simpleObstaclePlacementFlow_->StartDisappear();
	
}

void SikouteiDevelopPhase::Update()
{
    Game::Camera::Update(c_main_);

    if (Game::IO::Key::IsJustPressed('R'))
    {
        glass_->SetVelocity(Vector3{});
        ableDrag_ = true;
        LoadObstacleData(0);
    }

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
                cameraSpherical_.theta = humanRotate[i];
                //if (currentGlassUserIndex_ > i) cameraTheta += (humanRotate[i] - humanRotate[i + 1]);
                //else if (currentGlassUserIndex_ < i) cameraTheta += (humanRotate[i] - humanRotate[i - 1]);
                currentGlassUserIndex_ = i;
                Game::Camera::Setter::SetThetaTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.theta), 1.0f, EaseType::OUT_BACK, c_main_);
                Game::Camera::Setter::SetDistanceTarget(5.0f, 0.2f, EaseType::LINEAR, c_main_);
                Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotate[currentGlassUserIndex_]);
                glassPos.y = 1.28f;
                glass_->SetTranslate(glassPos);
                glass_->SetVelocity(Vector3{});
                ableDrag_ = true;
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

    // オブジェクト更新
    glass_->Update(c_main_);
    cocktailWater_->SetTranslate(glass_->GetTranslate() + Vector3{ 0.0f,-0.09f,0.0f });
    cocktailWater_->Update(c_main_);
    table_->Update(c_main_);
  
    simpleObstaclePlacementFlow_->Update();
    for (int i = 0; i < obstacleCount; ++i) {
        obstacles_[i]->SetTranslate(simpleObstaclePlacementFlow_->GetPieces(i).transform.translate);
    }

    

    for (int32_t i = 0; i < obstacleCount; i++)
    {
        obstacles_[i]->Update(c_main_);
    }

    //コライダー更新
    if (isDebugDraw_) collisionManager_->DebugUpdate(c_main_);
    CheckColliders();

    // ショット
    if (ableDrag_)
    {
        // マウス移動量
        const Vector2 mouseDelta = Game::IO::Mouse::Get2DPositionDelta();

        if (!Game::IO::Mouse::IsHeld(0))
        {
            cameraSpherical_.phi += mouseDelta.y * mouseInsensitivity_;
            constexpr float limit = 60.0f;
            cameraSpherical_.phi = std::clamp(cameraSpherical_.phi, 0.0f, limit);
            Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.0f, EaseType::OUT_BACK, c_main_);
        }
        cameraSpherical_.theta -= mouseDelta.x * mouseInsensitivity_;
        if (cameraSpherical_.theta > humanRotate[currentGlassUserIndex_] + 60.0f) cameraSpherical_.theta = humanRotate[currentGlassUserIndex_] + 60.0f;
        if (cameraSpherical_.theta < humanRotate[currentGlassUserIndex_] - 60.0f) cameraSpherical_.theta = humanRotate[currentGlassUserIndex_] - 60.0f;
        Game::Camera::Setter::SetThetaTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.theta), 0.0f, EaseType::OUT_BACK, c_main_);

        if (Game::IO::Mouse::IsJustPressed(0))
        {
            dragStartPos_ = Game::IO::Mouse::Get2DPosition();
            velocity_ = Vector2(0.0f, 0.0f);
        }
        if (Game::IO::Mouse::IsHeld(0))
        {
            Vector2 dragVector = Game::IO::Mouse::Get2DPosition() - dragStartPos_;
            float dragLength = dragVector.Length();

            constexpr float kPowerScale = 0.05f; // 感度。要調整
            constexpr float kMaxSpeed = 20.0f;   // 上限。要調整

            if (dragLength > 1.0f)
            {
                float angle = std::atan2(dragVector.x, dragVector.y);

                Vector3 cameraDir = Game::Camera::Getter::GetCameraDirection(c_main_);
                cameraDir.y = 0.0f;
                cameraDir.Normalize();

                float power = std::clamp(dragLength * kPowerScale, 0.0f, kMaxSpeed);
                velocity_ = Vector2(cameraDir.x, cameraDir.z) * power;
            }
            else
            {
                velocity_ = Vector2(0.0f, 0.0f);
            }
        }
        if (Game::IO::Mouse::IsJustReleased(0))
        {
            glass_->SetVelocity(Vector3(velocity_.x, 0.0f, velocity_.y));
			cameraSpherical_.phi = 20.0f;

			Game::Camera::Setter::SetDistanceTarget(3.0f, 0.2f, EaseType::LINEAR, c_main_);
            Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.2f, EaseType::LINEAR, c_main_);
            ableDrag_ = false;
        }
    }
    else
    {
  //      Vector3 glassVel = glass_->GetVelocity();
		//Vector3 glassVel2dNormalized = Vector3(glassVel.x, 0.0f, glassVel.z).Normalized();
  //      Vector3 yawPttch = Game::Math::YawPitchFromDirection(glassVel2dNormalized);
  //      Game::Camera::Setter::SetThetaTarget(yawPttch.y, 0.0f, EaseType::OUT_BACK, c_main_);
    }


    Game::Camera::Setter::SetCenter(glass_->GetTranslate(), 0.0f, EaseType::OUT_BACK, c_main_);

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

    for (int32_t i = 0; i < 3; i++)
    {
        Matrix4x4 world = humanTransforms_[i].GetWorldMatrix();
        Matrix4x4 wvp = world * viewPro;
        Vector4 color = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

        human_[i]->SetCBufferData(0, ShaderType::VertexShader, &wvp);
        human_[i]->SetCBufferData(1, ShaderType::VertexShader, &world);
        human_[i]->SetCBufferData(0, ShaderType::PixelShader, &color);
        human_[i]->SetCBufferData(1, ShaderType::PixelShader, &white1x1);
        human_[i]->Draw();
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
        markers_[i]->Draw();
    }


    //テーブルの描画
    table_->Draw();

    prediction_->Draw();

	// 障害物の描画
    for (int32_t i = 0; i < obstacleCount; i++)
	{
		obstacles_[i]->Draw();
	}
	cocktailWater_->Draw();
    //グラスは半透明なので後に描画する
    glass_->Draw();

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
    //table_->DrawImGui();
    prediction_->DrawImGui();
    //collisionManager_->DebugImGui();
    uiManager_->DebugImGui();

    ImGui::Begin("camera");

    if (ImGui::DragFloat("theta", &cameraSpherical_.theta, 0.1f, -180.0f, 180.0f))
    {
        Game::Camera::Setter::SetThetaTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.theta), 0.0f, EaseType::OUT_BACK, c_main_);
    }
    if (ImGui::DragFloat("phi", &cameraSpherical_.phi, 0.1f, -89.0f, 89.0f))
    {
        Game::Camera::Setter::SetPhiTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.phi), 0.0f, EaseType::OUT_BACK, c_main_);
    }
    if (ImGui::DragFloat("radius", &cameraSpherical_.radius, 0.1f, 0.1f, 100.0f))
    {
        Game::Camera::Setter::SetDistanceTarget(cameraSpherical_.radius, 0.0f, EaseType::OUT_BACK, c_main_);
    }

    ImGui::End();

    ImGui::Begin("glass");

    Vector3 glassVel = glass_->GetVelocity();
    Vector3 glassVel2dNormalized = Vector3(glassVel.x, 0.0f, glassVel.z).Normalized();
    Vector3 yawPttch = Game::Math::YawPitchFromDirection(glassVel2dNormalized);
	ImGui::Text("glass yawPitch: %f, %f, %f", yawPttch.x, yawPttch.y, yawPttch.z);
    ImGui::End();

    ImGui::Begin("Editor");

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
        static GlassType glassType;

        auto names = magic_enum::enum_names<GlassType>();
        auto values = magic_enum::enum_values<GlassType>();

        size_t current = magic_enum::enum_index(glassType).value();

        if (ImGui::BeginCombo("GlassType", names[current].data()))
        {
            for (std::size_t i = 0; i < names.size(); i++)
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
            obstacles_[obstacleCount]->SetTranslate(pos);
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
                    obstacles_[i]->SetTranslate({ pos2D.x, pos.y, pos2D.y });
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
        if (ImGui::DragFloat3("HumanRotate", humanRotate, 1.0f, -360.0f, 720.0f)) edit = true;
        if (ImGui::DragFloat("HumanScale", &humansize_, 1.0f, 0.0f, 120.0f)) edit = true;

        if (edit)
        {
            Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotate[0]);
            glassPos.y = 1.28f;
            glass_->SetTranslate(glassPos);

            for (int32_t i = 0; i < 3; i++)
            {
                humanTransforms_[i].translate = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 1.2f, humanRotate[i]);
                humanTransforms_[i].translate.y = 1.28f;
                humanTransforms_[i].scale = Vector3{ 0.5f,0.5f,0.5f };
            }

            for (int32_t i = 0; i < 6; i++)
            {
                float hugou = i % 2 == 0 ? -1.0f : 1.0f;
                float angle = humanRotate[(i / 2)] + (hugou * humansize_ * 0.5f);
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

    for (auto& prediction : prediction_->GetColliders()) {
        collisionManager_->AddCollider(prediction.get());
    }

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
	humanRotate[0] = humanRotateDeg.x;
	humanRotate[1] = humanRotateDeg.y;
	humanRotate[2] = humanRotateDeg.z;
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";
    JsonManager::Load(path, key, humansize_);

    Vector3 glassPos = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.5f, humanRotate[0]);
    glassPos.y = 1.28f;
    glass_->SetTranslate(glassPos);

    for (int32_t i = 0; i < 3; i++)
    {
        humanTransforms_[i].translate = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 1.2f, humanRotate[i]);
        humanTransforms_[i].translate.y = 1.28f;
        humanTransforms_[i].scale = Vector3{ 0.5f,0.5f,0.5f };
    }

    for (int32_t i = 0; i < 6; i++)
    {
        float hugou = i % 2 == 0 ? -1.0f : 1.0f;
        float angle = humanRotate[(i / 2)] + (hugou * humansize_ * 0.5f);
        markerAngles_[i] = angle;
    }
    for (int32_t i = 0; i < 6; i++)
    {
        markerTransforms_[i].translate = GetPositionOnCircle(table_->GetTranslate(), table_->GetRadius() * 0.8f, markerAngles_[i]);
        markerTransforms_[i].translate.y = 1.28f;
        markerTransforms_[i].scale = Vector3{ 0.1f,0.1f,0.1f };
    }

    cameraSpherical_.theta = humanRotate[0];
    Game::Camera::Setter::SetThetaTarget(Game::Math::Converter::DegreeToRadian(cameraSpherical_.theta), 0.2f, EaseType::OUT_BACK, c_main_);

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
	Vector3 humanRotateDeg = { humanRotate[0], humanRotate[1], humanRotate[2] };
    JsonManager::AddParam(path, key, humanRotateDeg);
    key = "/Stage" + std::to_string(stage) + "/Human/Scale";
    JsonManager::AddParam(path, key, humansize_);

	JsonManager::Save(path);
}
