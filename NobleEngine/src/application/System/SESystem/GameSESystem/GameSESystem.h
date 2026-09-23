#pragma once
#include"../ISESystem.h"
#include<functional>
class GameSESystem:public ISESystem
{
public:

    enum CommonSEs {
        SLIDE,
        DECIDE,
        DIVE,
        BREAK,
        MaxCommonSEs
    };

    enum GameSEs {
        MONEY = MaxCommonSEs,
        UI_BREAK,
        MaxGameSEs,
    };

    enum TitleSEs {
        ICE = MaxGameSEs,
        LIGHT,
        WATER,//まだ使ってない
        MAX_Title_SE
    };

    enum ResultSEs {
        LIQUID = MAX_Title_SE,
        MAX_SEs
    };

 

    enum Scene {
        Title,
        Game,
        Result,
        Common,
    };

    static void Load(const Scene scene);
    static void LoadCommon();

    static void Set();

    ~GameSESystem();
private:
    static std::unordered_map<Scene,std::function<void()>>LoadFunctions_;
    static void LoadTitle();
    static void LoadGame();
    static void LoadResult();
};

