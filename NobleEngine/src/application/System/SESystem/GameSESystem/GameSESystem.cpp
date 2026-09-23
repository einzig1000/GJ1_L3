#include "GameSESystem.h"
std::unordered_map<GameSESystem::Scene, std::function<void()>>GameSESystem::LoadFunctions_;

void GameSESystem::Load(Scene scene)
{
    auto it = LoadFunctions_.find(scene);
    if (it != LoadFunctions_.end())
    {
        it->second(); // 関数実行
    }
}

void GameSESystem::LoadCommon()
{
    gameAudios_.resize(MAX_SEs);
    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Title/";

    gameAudios_[CommonSEs::DECIDE].id = Game::Asset::Audio::Load(directoryPath + "Decide.mp3");
    gameAudios_[CommonSEs::DIVE].id = Game::Asset::Audio::Load(directoryPath + "Dive.mp3");
    gameAudios_[CommonSEs::SLIDE].id = Game::Asset::Audio::Load(directoryPath + "Slide.mp3");

    directoryPath = "assets/application/audio/SE・BGM/SE/Game/";
    //重複あり　BREAK
    gameAudios_[CommonSEs::BREAK].id = Game::Asset::Audio::Load(directoryPath + "Break.mp3");
}

void GameSESystem::Set()
{
    GameSESystem::LoadFunctions_[GameSESystem::Title] = &GameSESystem::LoadTitle;
    GameSESystem::LoadFunctions_[GameSESystem::Common] = &GameSESystem::LoadCommon;
    GameSESystem::LoadFunctions_[GameSESystem::Result] = &GameSESystem::LoadResult;
    GameSESystem::LoadFunctions_[GameSESystem::Game] = &GameSESystem::LoadGame;
}

GameSESystem::~GameSESystem()
{
    for (auto& audio : gameAudios_) {
        Game::Audio::StopAudio(audio.status_);
    }
}


void GameSESystem::LoadTitle()
{

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Title/";
    gameAudios_[TitleSEs::ICE].id = Game::Asset::Audio::Load(directoryPath + "Ice.mp3");
    gameAudios_[TitleSEs::LIGHT].id = Game::Asset::Audio::Load(directoryPath + "Light.mp3");
    gameAudios_[TitleSEs::WATER].id = Game::Asset::Audio::Load(directoryPath + "Water.mp3");
}

void GameSESystem::LoadGame()
{

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Game/";
    gameAudios_[GameSEs::MONEY].id = Game::Asset::Audio::Load(directoryPath + "Money.mp3");
    gameAudios_[GameSEs::UI_BREAK].id = Game::Asset::Audio::Load(directoryPath + "UI_Break.mp3");
}

void GameSESystem::LoadResult()
{

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Result/";
    gameAudios_[ResultSEs::LIQUID].id = Game::Asset::Audio::Load(directoryPath + "Liquid.mp3");
}
