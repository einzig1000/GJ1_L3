#include "SikouteiDevelopPhase.h"
#include <GameObject/Glass/Glass.h>
#include <GameObject/TableObject/TableObject.h>
#include <GameObject/Table/Table.h>
#include <System/CollisionManager/CollisionManager.h>
#include <Utilities/Json/JsonManager.h>
#include <externals/MagicEnum/magic_enum.hpp>
#include <numbers>

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
            return 0.0f; // 中心同士が重なっていて方向が定義できない特殊ケース
        }

        float angleRad = std::atan2(dir.y, dir.x);
        float angleDeg = angleRad * (180.0f / std::numbers::pi_v<float>);
        if (angleDeg < 0.0f) angleDeg += 360.0f;
        return angleDeg; // 0〜360
    }

    // その角度が指定範囲に入っているか
    bool IsInAngleRange(float angleDeg, float fromDeg, float toDeg)
    {
        if (fromDeg <= toDeg)
        {
            return angleDeg >= fromDeg && angleDeg < toDeg;
        }
        return angleDeg >= fromDeg || angleDeg < toDeg;
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
    table_ = std::make_unique<Table>();
    glass_ = std::make_unique<Glass>();
    for (int32_t i = 0; i < Constexprs::kMaxObstacleCount; i++)
    {
        obstacles_[i] = std::make_unique<TableObject>();
        obstacles_[i]->Initialize();
    }
}

SikouteiDevelopPhase::~SikouteiDevelopPhase()
{}

void SikouteiDevelopPhase::Initialize()
{
	// オブジェクト初期化
    glass_->Initialize();
    glass_->SetTranslate(Vector3{ 0.0f,1.28f,1.7f });
    //glass_->SetGlassTypeAndLoadModels(GlassType::Glass);
    table_->Initialize();
	for (int32_t i = 0; i < Constexprs::kMaxObstacleCount; i++)
	{
		obstacles_[i]->Initialize();
	}

	LoadObstacleData(0);
}

void SikouteiDevelopPhase::Update()
{
    Game::Camera::Update(c_main_);

    if (Game::IO::Key::IsJustPressed('R'))
    {
        Initialize();
    }

    if (deleteIndex >= 0)
    {
        obstacles_[deleteIndex] = std::move(obstacles_[obstacleCount - 1]);
        obstacleCount--;
		deleteIndex = -1;
    }

    glass_->Update(c_main_);
    table_->Update(c_main_);
    for (int32_t i = 0; i < obstacleCount; i++)
	{
		obstacles_[i]->Update(c_main_);
	}

    //コライダー描画のための更新
    if (isDebugDraw_) collisionManager_->DebugUpdate(c_main_);



    // テスト
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
            float angle = std::atan2(-dragVector.x, -dragVector.y);

            Vector3 cameraDir = Game::Camera::Getter::GetCameraDirection(c_main_);
            cameraDir.y = 0.0f;
            cameraDir.Normalize();

            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            Vector3 shotDir = Vector3(
                cameraDir.x * cosA - cameraDir.z * sinA,
                0.0f,
                cameraDir.x * sinA + cameraDir.z * cosA
            );

            float power = std::clamp(dragLength * kPowerScale, 0.0f, kMaxSpeed);
            velocity_ = Vector2(shotDir.x, shotDir.z) * power;
        }
        else
        {
            velocity_ = Vector2(0.0f, 0.0f);
        }
    }
    if (Game::IO::Key::IsHeld(VK_LSHIFT) && Game::IO::Mouse::IsJustReleased(0))
    {
        glass_->SetVelocity(Vector3(velocity_.x, 0.0f, velocity_.y));
    }

	Vector2 tablePos2D = Vector2(table_->GetTranslate().x, table_->GetTranslate().z);
    if (IsTouchingInnerEdge(tablePos2D, table_->GetRadius(), glass_->(), glass_->GetRadius()))
    {
        float angle = GetContactAngleDeg(big.center, small.center);
        if (IsInAngleRange(angle, 30.0f, 60.0f))
        {
            // 30°〜60°のセグメントにヒット
        }
    }


    CheckColliders();
}

void SikouteiDevelopPhase::Draw()
{
    //テーブルの描画
    table_->Draw();
	// 障害物の描画
    for (int32_t i = 0; i < obstacleCount; i++)
	{
		obstacles_[i]->Draw();
	}
    //グラスは半透明なので後に描画する
    glass_->Draw();

    //コライダーデバック描画
    if (isDebugDraw_) collisionManager_->DebugDraw();
}

void SikouteiDevelopPhase::DrawImGui()
{
    glass_->DrawImGui();
	//for (auto& obstacle : obstacles_)
	//{
	//	obstacle->DrawImGui();
	//}
    obstacles_[0]->DrawImGui();
    //table_->DrawImGui();
    //collisionManager_->DebugImGui();


    ImGui::Begin("StageEditor");

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

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}

bool SikouteiDevelopPhase::LoadObstacleData(int32_t stage)
{
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string countKey = "/Stage" + std::to_string(stage) + "/Count";
	bool success = JsonManager::Load(path, countKey, obstacleCount);
	if (!success) return false;
    for (int32_t i = 0; i < obstacleCount; i++)
    {
        obstacles_[i]->Initialize();

        std::string key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
		std::string typeStr;
        JsonManager::Load(path, key, typeStr);
		obstacles_[i]->SetGlassTypeAndLoadModels(magic_enum::enum_cast<GlassType>(typeStr).value());

        key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
        Vector3 translate;
        JsonManager::Load(path, key, translate);
		obstacles_[i]->SetTranslate(translate);
    }

	return true;
}

void SikouteiDevelopPhase::SaveObstacleData(int32_t stage)
{
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string countKey = "/Stage" + std::to_string(stage) + "/Count";
	JsonManager::AddParam(path, countKey, obstacleCount);
	for (int32_t i = 0; i < obstacleCount; i++)
	{
		std::string key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
		JsonManager::AddParam(path, key, magic_enum::enum_name(obstacles_[i]->GetGlassType()));

		key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
		JsonManager::AddParam(path, key, obstacles_[i]->GetTranslate());
	}

	JsonManager::Save(path);
}
