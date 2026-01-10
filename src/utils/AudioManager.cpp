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
        SetMusicVolume(gameMusic, 0.5f);
    }

    // Load sounds
    sounds[SoundId::March] = LoadSound(FileSystem::getPath("res/sounds/march.wav").c_str());
    sounds[SoundId::ArtilleryAttack] = LoadSound(FileSystem::getPath("res/sounds/artillery_attack.wav").c_str());
	sounds[SoundId::NormalAttack] = LoadSound(FileSystem::getPath("res/sounds/normal_attack.wav").c_str());
	sounds[SoundId::ButtonClick] = LoadSound(FileSystem::getPath("res/sounds/button_click.wav").c_str());
    sounds[SoundId::Victory] = LoadSound(FileSystem::getPath("res/sounds/victory.wav").c_str());
    sounds[SoundId::Defeat] = LoadSound(FileSystem::getPath("res/sounds/defeat.wav").c_str());
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

void AudioManager::PlayMusic()
{
	if (musicPlaying)
	{
        PlayMusicStream(gameMusic);
	}
}

void AudioManager::StopMusic()
{
    if (musicPlaying)
    {
        StopMusicStream(gameMusic);
	}
}

void AudioManager::Play(SoundId id, float volume)
{
    Sound& s = sounds.at(id);
    SetSoundVolume(s, volume);
	PlaySound(s);
}
