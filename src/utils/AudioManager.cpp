#include "AudioManager.hpp"

#include "Filesystem.hpp"

AudioManager& AudioManager::getInstance()
{
    static AudioManager instance;
    return instance;
}

void AudioManager::Init(bool playMusic)
{
    InitAudioDevice();

    musicPlaying = playMusic;

    if (playMusic)
    {
        gameMusic = LoadMusicStream(FileSystem::getPath("res/sounds/game_music.mp3").c_str());
        PlayMusicStream(gameMusic);
        SetMusicVolume(gameMusic, 0.5f);
    }

    // Load sounds
    sounds[SoundId::March] = LoadSound(FileSystem::getPath("res/sounds/infantry_march.wav").c_str());
    sounds[SoundId::ArtilleryAttack] = LoadSound(FileSystem::getPath("res/sounds/artillery_attack.wav").c_str());
    sounds[SoundId::BaseDestroyed] = LoadSound(FileSystem::getPath("res/sounds/base_destroyed.wav").c_str());
    sounds[SoundId::UIClick] = LoadSound(FileSystem::getPath("res/sounds/ui_click.wav").c_str());
}

void AudioManager::Update()
{
    if (musicPlaying)
    {
        UpdateMusicStream(gameMusic);
    }
}

void AudioManager::Shutdown()
{
    for (auto& pair : sounds)
    {
        UnloadSound(pair.second);
    }
    CloseAudioDevice();
}

void AudioManager::Play(SoundId id, float volume)
{
    Sound& s = sounds.at(id);
    SetSoundVolume(s, volume);

    static float lastPlay = 0.0f;
    float now = GetTime();

    if (now - lastPlay > threshhold) // prevent spamming sounds{
    {
        PlaySound(s);
        lastPlay = now;
    }
}
