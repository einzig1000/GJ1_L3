#pragma once
#include<Game.h>
#include<array>

class GameBGMSystem
{
public:
    struct GameAudio {
        int32_t id = 0;
        float volume = 0.0f;
        int32_t status_ = -1;
    };

    enum BGMs {
        TITLE_BGM,
        GAME_BGM,
        RESULT_WIN,
        RESULT_LOSE,
        MAX_BGM
    };

public:

    static GameBGMSystem& GetInstance() {
        static GameBGMSystem instance;
        return instance;
    }


    // コピー・ムーブ禁止
    GameBGMSystem(const GameBGMSystem&) = delete;
    GameBGMSystem& operator=(const GameBGMSystem&) = delete;
    GameBGMSystem(GameBGMSystem&&) = delete;
    GameBGMSystem& operator=(GameBGMSystem&&) = delete;

    void Load();
    void StopAllAudio();
    void Initialize(const BGMs& bgm);
    void UpVolume(const BGMs& bgm);
    void DownVolume(const BGMs& bgm);
private:
    //コンストラクタデストラクタの隠蔽
    GameBGMSystem() = default;
    ~GameBGMSystem() = default;
  

private:
    std::array<GameAudio, MAX_BGM>bgmAudios_;

};

