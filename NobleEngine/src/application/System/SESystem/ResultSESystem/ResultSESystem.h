#pragma once
#include"../ISESystem.h"
class ResultSESystem :
    public ISESystem
{
    enum SEs {
        BREAK,
        DECIDE,
        DIVE,
        LIQUID,
        MAX_SE
    };

    void Load()override;
};

