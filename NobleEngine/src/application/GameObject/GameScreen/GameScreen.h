#pragma once
#include <Game.h>

class GameScreen
{
public:
    enum Layer {
    MAIN_SCREEN,
    UI_SCREEN,
    MAX_LAYER,
    };
private:
    int32_t renderTextureIDs_[MAX_LAYER] = {-1,-1};
    std::unique_ptr<RenderObject> drawForMain_[MAX_LAYER];

public:
    GameScreen();
    ~GameScreen();
    void Draw(const int32_t renderTargetID);
    int32_t GetRenderTextureID(Layer layer) { if (layer == MAX_LAYER) { return -1; }return  renderTextureIDs_[layer]; }
};

