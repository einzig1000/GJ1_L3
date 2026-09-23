#pragma once
#include<Game.h>

class ISESystem
{
public:
    struct GameAudio {
        int32_t id = 0;
        int32_t status_ = -1;
    };

public:
    virtual ~ISESystem() = default;
    template <typename T>
    requires std::is_enum_v<T>
    static void PlaySE(T type,bool isOriginal = false);
    template <typename T>
  requires std::is_enum_v<T>
    static void StopSE(T type);
   static std::vector<GameAudio>gameAudios_;
};

template<typename T>
requires std::is_enum_v<T>
inline void ISESystem::PlaySE(T type, bool isOriginal)
{
    if (gameAudios_.size() <= static_cast<size_t>(type)) {
        return;
    }
    if (isOriginal) {
        if (Game::Audio::IsAudioPlaying(gameAudios_[type].status_)) {
            return;
        }
    }
     gameAudios_[type].status_ = Game::Audio::PlayAudio(gameAudios_[static_cast<uint32_t>(type)].id,false);
}


template<typename T>
    requires std::is_enum_v<T>
inline void ISESystem::StopSE(T type)
{
    if (gameAudios_.size() <= static_cast<size_t>(type)) {
        return;
    }
    Game::Audio::StopAudio(gameAudios_[static_cast<int32_t>(type)].status_);

}
