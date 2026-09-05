#include "Audio.hpp"

#include <SDL.h>
#include <SDL_mixer.h>

#include <cstdio>

namespace flying {
namespace {

constexpr int kSfxVolume = MIX_MAX_VOLUME;
constexpr int kMusicVolume = 42;  // ~33% so flap/collide stay in front

}  // namespace

bool Audio::init(const std::string& assetsRoot, ProgressFn progress) {
    int step = 0;
    auto advance = [&]() {
        ++step;
        if (progress) {
            progress(step, kLoadSteps);
        }
    };

    const int want = MIX_INIT_OGG;
    if ((Mix_Init(want) & want) != want) {
        std::fprintf(stderr, "Aviso Mix_Init OGG: %s\n", Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        std::fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
        Mix_Quit();
        return false;
    }
    opened_ = true;

    Mix_AllocateChannels(16);
    Mix_Volume(-1, kSfxVolume);
    advance();

    const std::string flapPath = assetsRoot + "/flap.wav";
    const std::string collidePath = assetsRoot + "/collide.wav";
    const std::string passPath = assetsRoot + "/pass.wav";

    flap_ = Mix_LoadWAV(flapPath.c_str());
    if (!flap_) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", flapPath.c_str(), Mix_GetError());
    } else {
        Mix_VolumeChunk(flap_, kSfxVolume);
    }
    advance();

    collide_ = Mix_LoadWAV(collidePath.c_str());
    if (!collide_) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", collidePath.c_str(), Mix_GetError());
    } else {
        Mix_VolumeChunk(collide_, kSfxVolume);
    }
    advance();

    pass_ = Mix_LoadWAV(passPath.c_str());
    if (!pass_) {
        std::fprintf(stderr, "No se pudo cargar %s: %s\n", passPath.c_str(), Mix_GetError());
    } else {
        Mix_VolumeChunk(pass_, kSfxVolume);
    }
    advance();

    ready_ = (flap_ != nullptr) || (collide_ != nullptr) || (pass_ != nullptr);

    const std::string musicPath = assetsRoot + "/background.ogg";
    musicThread_ = std::thread([this, musicPath]() {
        if (shutdownRequested_.load()) {
            return;
        }
        music_ = Mix_LoadMUS(musicPath.c_str());
        if (music_) {
            if (shutdownRequested_.load()) {
                return;
            }
            Mix_VolumeMusic(kMusicVolume);
            if (Mix_PlayMusic(music_, -1) != 0) {
                std::fprintf(stderr, "Mix_PlayMusic: %s\n", Mix_GetError());
            }
        } else {
            musicChunk_ = Mix_LoadWAV(musicPath.c_str());
            if (!musicChunk_) {
                std::fprintf(stderr, "No se pudo cargar %s: %s\n",
                             musicPath.c_str(), Mix_GetError());
            } else if (!shutdownRequested_.load()) {
                Mix_VolumeChunk(musicChunk_, kMusicVolume);
                musicChannel_.store(Mix_PlayChannel(-1, musicChunk_, -1));
                if (musicChannel_.load() < 0) {
                    std::fprintf(stderr, "Mix_PlayChannel musica: %s\n", Mix_GetError());
                }
            }
        }
        musicReady_.store(true);
    });

    return ready_;
}

void Audio::shutdown() {
    shutdownRequested_.store(true);
    if (musicThread_.joinable()) {
        musicThread_.join();
    }

    Mix_HaltMusic();
    const int ch = musicChannel_.load();
    if (ch >= 0) {
        Mix_HaltChannel(ch);
        musicChannel_.store(-1);
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
    if (pass_) {
        Mix_FreeChunk(pass_);
        pass_ = nullptr;
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
            chunk = pass_;
            break;
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
        const int ch = musicChannel_.load();
        if (ch >= 0) {
            Mix_Pause(ch);
        }
    } else if (!muted_) {
        Mix_ResumeMusic();
        const int ch = musicChannel_.load();
        if (ch >= 0) {
            Mix_Resume(ch);
        }
    }
}

}  // namespace flying
