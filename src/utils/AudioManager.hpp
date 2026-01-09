#pragma once

#include <raylib.h>
#include <unordered_map>

enum class SoundId { // different sound effects
    March,
    ArtilleryAttack,
    BaseDestroyed,
    UIClick,
    Victory,
    Defeat
};

class AudioManager
{
public: 
    static AudioManager& getInstance();

    void Init(bool playMusic = true);
    void Update();
    void Shutdown();

    void Play(SoundId id, float volume = 1.0f);
    

private:
    std::unordered_map<SoundId, Sound> sounds;

    Music gameMusic;
    bool musicPlaying = false;

    const float threshhold = 0.3f; // minimum time between same sound plays
};
