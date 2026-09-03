#include "Audio.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

#include <cstdio>

namespace flying {
namespace {

constexpr int kSfxVolume = MIX_MAX_VOLUME;
constexpr int kMusicVolume = 42;  // ~33% so flap/collide stay in front

}  // namespace

bool Audio::init(const std::string& assetsRoot) {
    const int want = MIX_INIT_FLAC;
    if ((Mix_Init(want) & want) != want) {
        std::fprintf(stderr, "Aviso Mix_Init FLAC: %s\n", Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        std::fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
        Mix_Quit();
        return false;
    }
    opened_ = true;

    Mix_AllocateChannels(16);
    Mix_Volume(-1, kSfxVolume);

    const std::string flapPath = assetsRoot + "/flap.flac";
    const std::string collidePath = assetsRoot + "/collide.wav";
    const std::string musicPath = assetsRoot + "/background.wav";

    flap_ = Mix_LoadWAV(flapPath.c_str());
    collide_ = Mix_LoadWAV(collidePath.c_str());
    if (!flap_) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", flapPath.c_str(), Mix_GetError());
    }
    if (!collide_) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", collidePath.c_str(), Mix_GetError());
    }
    if (flap_) {
        Mix_VolumeChunk(flap_, kSfxVolume);
    }
    if (collide_) {
        Mix_VolumeChunk(collide_, kSfxVolume);
    }

    music_ = Mix_LoadMUS(musicPath.c_str());
    if (music_) {
        Mix_VolumeMusic(kMusicVolume);
        if (Mix_PlayMusic(music_, -1) != 0) {
            std::fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
        }
    } else {
        musicChunk_ = Mix_LoadWAV(musicPath.c_str());
        if (!musicChunk_) {
            std::fprintf(stderr, "No se pudo cargar %s: %s\n", musicPath.c_str(), Mix_GetError());
        } else {
            Mix_VolumeChunk(musicChunk_, kMusicVolume);
            musicChannel_ = Mix_PlayChannel(-1, musicChunk_, -1);
            if (musicChannel_ < 0) {
                std::fprintf(stderr, "Mix_PlayChannel musica: %s\n", Mix_GetError());
            }
        }
    }

    ready_ = (flap_ != nullptr) || (collide_ != nullptr) || (music_ != nullptr) ||
             (musicChunk_ != nullptr);
    return ready_;
}

void Audio::shutdown() {
    Mix_HaltMusic();
    if (musicChannel_ >= 0) {
        Mix_HaltChannel(musicChannel_);
        musicChannel_ = -1;
    }
    if (music_) {
        Mix_FreeMusic(music_);
        music_ = nullptr;
    }
    if (musicChunk_) {
        Mix_FreeChunk(musicChunk_);
        musicChunk_ = nullptr;
    }
    if (flap_) {
        Mix_FreeChunk(flap_);
        flap_ = nullptr;
    }
    if (collide_) {
        Mix_FreeChunk(collide_);
        collide_ = nullptr;
    }
    if (opened_) {
        Mix_CloseAudio();
        opened_ = false;
    }
    Mix_Quit();
    ready_ = false;
}

void Audio::play(SoundId id) {
    if (!ready_ || muted_) {
        return;
    }
    Mix_Chunk* chunk = nullptr;
    switch (id) {
        case SoundId::Flap:
            chunk = flap_;
            break;
        case SoundId::Hit:
            chunk = collide_;
            break;
        case SoundId::Score:
        case SoundId::Die:
            break;
    }
    if (!chunk) {
        return;
    }
    Mix_PlayChannel(-1, chunk, 0);
}

void Audio::setMuted(bool muted) {
    muted_ = muted;
    setPaused(muted);
}

void Audio::setPaused(bool paused) {
    if (paused) {
        Mix_PauseMusic();
        if (musicChannel_ >= 0) {
            Mix_Pause(musicChannel_);
        }
    } else if (!muted_) {
        Mix_ResumeMusic();
        if (musicChannel_ >= 0) {
            Mix_Resume(musicChannel_);
        }
    }
}

}  // namespace flying
