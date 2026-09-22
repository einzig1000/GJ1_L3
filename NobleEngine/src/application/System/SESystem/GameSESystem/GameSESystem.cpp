#include "GameSESystem.h"

void GameSESystem::Load()
{
    gameAudios_.resize(MAX_SE);

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Game/";
    //重複あり　BREAK
    gameAudios_[BREAK].id = Game::Asset::Audio::Load(directoryPath+"Break.mp3");
    gameAudios_[MONEY].id =Game::Asset::Audio::Load(directoryPath + "Money.mp3");
    gameAudios_[SLIDE].id =Game::Asset::Audio::Load(directoryPath + "Slide.mp3");
    gameAudios_[UI_BREAK].id =Game::Asset::Audio::Load(directoryPath + "UI_Break.mp3");
}
