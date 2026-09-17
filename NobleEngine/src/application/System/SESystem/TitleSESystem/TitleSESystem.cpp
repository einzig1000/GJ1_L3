#include "TitleSESystem.h"

void TitleSESystem::Load()
{

    gameAudios_.resize(MAX_SE);

    std::string directoryPath = "assets/application/audio/SE・BGM/SE/Title/";
    //重複あり　DECIDE　DIVE
    gameAudios_[DECIDE].id = Game::Asset::Audio::Load(directoryPath + "Decide.mp3");
    gameAudios_[DIVE].id = Game::Asset::Audio::Load(directoryPath + "Dive.mp3");
    gameAudios_[ICE].id = Game::Asset::Audio::Load(directoryPath + "Ice.mp3");
    gameAudios_[LIGHT].id = Game::Asset::Audio::Load(directoryPath + "Light.mp3");
    gameAudios_[SLIDE].id = Game::Asset::Audio::Load(directoryPath + "Slide.mp3");
    gameAudios_[WATER].id = Game::Asset::Audio::Load(directoryPath + "Water.mp3");
}
//
//TitleSESystem::~TitleSESystem()
//{
//    for (auto& audio : gameAudios_) {
//        Game::Audio::StopAudio(audio.id);
//    }
//
//}
