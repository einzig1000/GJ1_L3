#include "GameLight.h"
#include <Utilities/Json/JsonManager.h>

bool GameLight::Load()
{
    std::string path = "assets/application/json/StageData/Lights.json";
    std::string key = "/Count";
    bool success = JsonManager::Load(path, key, data_.LightCount);
    if (!success) return false;

    key = "/ambientColor";
    success = JsonManager::Load(path, key, data_.ambientColor);
    if (!success) return false;

    for (int32_t i = 0; i < data_.LightCount; i++)
    {
        key = "/Light" + std::to_string(i) + "/color";
        JsonManager::Load(path, key, data_.lights[i].color);
        key = "/Light" + std::to_string(i) + "/intensity";
        JsonManager::Load(path, key, data_.lights[i].intensity);
        key = "/Light" + std::to_string(i) + "/direction";
        JsonManager::Load(path, key, data_.lights[i].direction);
        key = "/Light" + std::to_string(i) + "/spotCos";
        JsonManager::Load(path, key, data_.lights[i].spotCos);
        key = "/Light" + std::to_string(i) + "/position";
        JsonManager::Load(path, key, data_.lights[i].position);
        key = "/Light" + std::to_string(i) + "/range";
        JsonManager::Load(path, key, data_.lights[i].range);
        key = "/Light" + std::to_string(i) + "/position";
        JsonManager::Load(path, key, data_.lights[i].position);
        key = "/Light" + std::to_string(i) + "/type";
        JsonManager::Load(path, key, data_.lights[i].type);
    }

    return true;
}

void GameLight::Save()
{
    std::string path = "assets/application/json/StageData/Lights.json";
    std::string key = "/Count";
    JsonManager::AddParam(path, key, data_.LightCount);
    key = "/ambientColor";
    JsonManager::AddParam(path, key, data_.ambientColor);

    for (int32_t i = 0; i < data_.LightCount; i++)
    {
        key = "/Light" + std::to_string(i) + "/color";
        JsonManager::AddParam(path, key, data_.lights[i].color);
        key = "/Light" + std::to_string(i) + "/intensity";
        JsonManager::AddParam(path, key, data_.lights[i].intensity);
        key = "/Light" + std::to_string(i) + "/direction";
        JsonManager::AddParam(path, key, data_.lights[i].direction);
        key = "/Light" + std::to_string(i) + "/spotCos";
        JsonManager::AddParam(path, key, data_.lights[i].spotCos);
        key = "/Light" + std::to_string(i) + "/position";
        JsonManager::AddParam(path, key, data_.lights[i].position);
        key = "/Light" + std::to_string(i) + "/range";
        JsonManager::AddParam(path, key, data_.lights[i].range);
        key = "/Light" + std::to_string(i) + "/type";
        JsonManager::AddParam(path, key, data_.lights[i].type);
    }
    JsonManager::Save(path);
}

void GameLight::DrawImGui()
{

    // ライト
    if (ImGui::TreeNode("LightData"))
    {
        if (ImGui::Button("Load", ImVec2(40, 20))) Load();
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(40, 20))) Save();

        ImGui::DragInt("LightCount", &data_.LightCount, 1, 0, 4);
        ImGui::ColorEdit3("ambientColor", &data_.ambientColor.x);
        for (int i = 0; i < data_.LightCount; ++i)
        {

            std::string lightNodeName = "Light" + std::to_string(i);
            if (ImGui::TreeNode(lightNodeName.c_str()))
            {
                ImGui::SeparatorText("Common");
                ImGui::ColorEdit3("color", &data_.lights[i].color.x);
                ImGui::DragFloat("intensity", &data_.lights[i].intensity, 0.01f);

                ImGui::SeparatorText("type");
                ImGui::DragInt("type", &data_.lights[i].type, 1, 0, 2);

                ImGui::SeparatorText("Directional");
                ImGui::DragFloat3("direction", &data_.lights[i].direction.x, 0.01f);

                ImGui::SeparatorText("Spot");
                ImGui::DragFloat("spotCos", &data_.lights[i].spotCos, 0.01f);

                ImGui::SeparatorText("Point / Spot");
                ImGui::DragFloat3("position", &data_.lights[i].position.x, 0.01f);
                ImGui::DragFloat("range", &data_.lights[i].range, 0.01f);

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }




}
