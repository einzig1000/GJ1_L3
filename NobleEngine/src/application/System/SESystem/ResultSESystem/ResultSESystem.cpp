#include "ResultSESystem.h"

void ResultSESystem::Load()
{
    gameAudios_.resize(MAX_SE);

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Result/";
    //重複あり　BREAK
    gameAudios_[BREAK].id = Game::Asset::Audio::Load(directoryPath + "Break.mp3");
    //重複あり　DECIDE　DIVE
    gameAudios_[DECIDE].id = Game::Asset::Audio::Load(directoryPath + "Decide.mp3");
    gameAudios_[DIVE].id = Game::Asset::Audio::Load(directoryPath + "Dive.mp3");

    gameAudios_[LIQUID].id = Game::Asset::Audio::Load(directoryPath + "Liquid.mp3");
}
