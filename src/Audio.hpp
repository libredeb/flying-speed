#pragma once

#include <string>

struct Mix_Chunk;
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

namespace flying {

enum class SoundId {
    Flap,
    Score,
    Hit,
    Die,
};

class Audio {
public:
    bool init(const std::string& assetsRoot);
    void shutdown();
    void play(SoundId id);
    void setMuted(bool muted);
    void setPaused(bool paused);
    bool muted() const { return muted_; }

private:
    Mix_Chunk* flap_ = nullptr;
    Mix_Chunk* collide_ = nullptr;
    Mix_Music* music_ = nullptr;
    Mix_Chunk* musicChunk_ = nullptr;
    int musicChannel_ = -1;
    bool muted_ = false;
    bool ready_ = false;
    bool opened_ = false;
};

}  // namespace flying
