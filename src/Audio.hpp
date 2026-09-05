#pragma once

#include "Assets.hpp"

#include <SDL_mixer.h>

#include <atomic>
#include <string>
#include <thread>

namespace flying {

enum class SoundId {
    Flap,
    Score,
    Hit,
    Die,
};

class Audio {
public:
    static constexpr int kLoadSteps = 4;

    bool init(const std::string& assetsRoot, ProgressFn progress = nullptr);
    void shutdown();
    void play(SoundId id);
    void setMuted(bool muted);
    void setPaused(bool paused);
    bool muted() const { return muted_; }

private:
    Mix_Chunk* flap_ = nullptr;
    Mix_Chunk* collide_ = nullptr;
    Mix_Chunk* pass_ = nullptr;
    Mix_Music* music_ = nullptr;
    Mix_Chunk* musicChunk_ = nullptr;
    std::atomic<int> musicChannel_{-1};
    bool muted_ = false;
    bool ready_ = false;
    bool opened_ = false;

    std::thread musicThread_;
    std::atomic<bool> musicReady_{false};
    std::atomic<bool> shutdownRequested_{false};
};

}  // namespace flying
