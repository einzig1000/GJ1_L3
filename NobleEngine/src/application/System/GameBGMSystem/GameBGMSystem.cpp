#include "GameBGMSystem.h"
#include<algorithm>

void GameBGMSystem::Load()
{
    bgmAudios_[TITLE_BGM].id = Game::Asset::Audio::Load("assets/application/audio/BGM/TitleScene.mp3");
    bgmAudios_[GAME_BGM].id = Game::Asset::Audio::Load("assets/application/audio/BGM/GameScene.mp3");

    bgmAudios_[RESULT_WIN].id = Game::Asset::Audio::Load("assets/application/audio/BGM/ResultScene_Win.mp3");
    bgmAudios_[RESULT_LOSE].id = Game::Asset::Audio::Load("assets/application/audio/BGM/ResultScene_Lose.mp3");
}

void GameBGMSystem::StopAllAudio()
{
    for (auto& audio : bgmAudios_) {
        Game::Audio::StopAudio(audio.id);
    }
}

void GameBGMSystem::Initialize(const BGMs& bgm)
{
    if (bgm == MAX_BGM) {
        return;
    }
    
    bgmAudios_[bgm].volume = 0.0f;
    bgmAudios_[bgm].status_ = -1;
    bgmAudios_[bgm].status_ =  Game::Audio::PlayAudio(bgmAudios_[bgm].id, true, bgmAudios_[bgm].volume);
}

void GameBGMSystem::UpVolume(const BGMs& bgm)
{
    if (bgm == MAX_BGM) {
        return;
    }
    bgmAudios_[bgm].volume += Game::Time::GetScaledDeltaTimeMs() * 0.0001f;
    bgmAudios_[bgm].volume = std::clamp(bgmAudios_[bgm].volume, 0.0f, 1.0f);
    Game::Audio::SetAudioVolume(bgmAudios_[bgm].status_, bgmAudios_[bgm].volume);
}

void GameBGMSystem::DownVolume(const BGMs& bgm)
{
    if (bgm == MAX_BGM) {
        return;
    }

    bgmAudios_[bgm].volume -= Game::Time::GetScaledDeltaTimeMs() * 0.001f;

    if (bgmAudios_[bgm].volume > 0.0f) {
        Game::Audio::SetAudioVolume(bgmAudios_[bgm].status_, bgmAudios_[bgm].volume);
    } else {
        bgmAudios_[bgm].volume = 0.0f;
        Game::Audio::StopAudio(bgmAudios_[bgm].id);
    }
}

