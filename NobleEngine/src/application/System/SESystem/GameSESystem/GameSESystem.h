#pragma once
#include"../ISESystem.h"
class GameSESystem:public ISESystem
{
public:
    enum SEs {
        BREAK,
        MONEY,
        SLIDE,
        UI_BREAK,
        MAX_SE
    };

    void Load()override;
};

