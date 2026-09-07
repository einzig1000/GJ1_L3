#include "SikouteiDevelopPhase.h"
#include <GameObject/Glass/Glass.h>
#include <GameObject/Obstacle/Obstacle.h>
#include <GameObject/Table/Table.h>
#include <System/CollisionManager/CollisionManager.h>
#include <Utilities/Json/JsonManager.h>
#include <externals/MagicEnum/magic_enum.hpp>
#include<GameObject/PredictionObj/PredictionObj.h>

SikouteiDevelopPhase::SikouteiDevelopPhase()
{
    // カメラ
    c_main_ = Game::Camera::AddCamera("SikouteiDevelopPhase");

    //コリジョン管理
    collisionManager_->Load();
    collisionManager_ = std::make_unique<CollisionManager>();


    // オブジェクト実体生成
    glass_ = std::make_unique<Glass>();
    table_ = std::make_unique<Table>();
    predictionObj_ = std::make_unique<PredictionObj>();
}

SikouteiDevelopPhase::~SikouteiDevelopPhase()
{}

void SikouteiDevelopPhase::Initialize()
{
	// オブジェクト初期化
    glass_->Initialize();
    table_->Initialize();
    //予測オブジェクト
    predictionObj_->Initialize();
	for (auto& obstacle : obstacles_)
	{
		obstacle->Initialize();
	}
	LoadObstacleData(0);
}

void SikouteiDevelopPhase::Update()
{
    Game::Camera::Update(c_main_);

    if (deleteIndex >= 0)
    {
        obstacles_[deleteIndex] = std::move(obstacles_.back());
        obstacles_.pop_back();
		deleteIndex = -1;
    }

    glass_->Update(c_main_);
    table_->Update(c_main_);
	for (auto& obstacle : obstacles_)
	{
		obstacle->Update(c_main_);
	}


    //コライダー描画のための更新
    if (DisebugDraw_) collisionManager_->DebugUpdate(c_main_);



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
    if (Game::IO::Mouse::IsJustReleased(0))
    {
        glass_->SetVelocity(Vector3(velocity_.x, 0.0f, velocity_.y));
    }

   
    //予測オブジェクト
    predictionObj_->SetTranslate(glass_->GetTranslate());
    Vector3 glassNormalVel = Vector3(velocity_.x, 0.0f, velocity_.y);
    predictionObj_->SetNormal(glassNormalVel);
    predictionObj_->Update(c_main_);

    CheckColliders();
}

void SikouteiDevelopPhase::Draw()
{
    //テーブルの描画
    table_->Draw();
	// 障害物の描画
	for (auto& obstacle : obstacles_)
	{
		obstacle->Draw();
	}
    //予測オブジェクト
    predictionObj_->Draw();

    //グラスは半透明なので後に描画する
    glass_->Draw();

    //コライダーデバック描画
    if (DisebugDraw_) collisionManager_->DebugDraw();
}

void SikouteiDevelopPhase::DrawImGui()
{
    glass_->DrawImGui();
	for (auto& obstacle : obstacles_)
	{
		obstacle->DrawImGui();
	}
    //table_->DrawImGui();
    //collisionManager_->DebugImGui();
    predictionObj_->DrawImGui();

    ImGui::Begin("StageEditor");

    ImGui::Checkbox("DebugDraw", &DisebugDraw_);

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

        int current = magic_enum::enum_index(glassType).value();

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
            obstacles_.push_back(std::make_unique<Obstacle>());
            obstacles_.back()->Initialize();
            obstacles_.back()->SetGlassTypeAndLoadModels(glassType);
            Vector3 pos = { position.x, 1.28f, position.y };
            obstacles_.back()->SetTranslate(pos);
        }

        ImGui::TreePop();
    }

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

    for (auto& prediction : predictionObj_->GetColliders()) {
        collisionManager_->AddCollider(prediction.get());
    }

    //コライダーをチェックする
    collisionManager_->CheckAllCollisions();
}

bool SikouteiDevelopPhase::LoadObstacleData(int32_t stage)
{
    int32_t count = 0;
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string countKey = "/Stage" + std::to_string(stage) + "/Count";
	bool success = JsonManager::Load(path, countKey, count);
	if (!success) return false;
	obstacles_.clear();
    obstacles_.resize(count);
    for (int32_t i = 0; i < count; i++)
    {
		obstacles_[i] = std::make_unique<Obstacle>();
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
	int32_t count = static_cast<int32_t>(obstacles_.size());
    std::string path = "assets/application/json/StageData/Obstacles.json";
    std::string countKey = "/Stage" + std::to_string(stage) + "/Count";
	JsonManager::AddParam(path, countKey, count);
	for (int32_t i = 0; i < count; i++)
	{
		std::string key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/type";
		JsonManager::AddParam(path, key, magic_enum::enum_name(obstacles_[i]->GetGlassType()));

		key = "/Stage" + std::to_string(stage) + "/Obstacle" + std::to_string(i) + "/translate";
		JsonManager::AddParam(path, key, obstacles_[i]->GetTranslate());
	}

	JsonManager::Save(path);
}
