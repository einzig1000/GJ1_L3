#pragma once
#include<Game.h>

class GameLight
{
private:
    LightDataForGPU data_;
public:
    bool Load();
    void Save();
    void DrawImGui();
    LightDataForGPU& GetLightData() { return data_; }
};

