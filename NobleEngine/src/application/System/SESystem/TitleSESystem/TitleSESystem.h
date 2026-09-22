#pragma once
#include"../ISESystem.h"
class TitleSESystem :public ISESystem
{
public:
    enum SEs {
        DECIDE,
        DIVE,
        ICE,//まだ使ってない
        LIGHT,
        SLIDE,//まだ使ってない
        WATER,//まだ使ってない
        MAX_SE
    };

    void Load()override;
    //~TitleSESystem();
};
