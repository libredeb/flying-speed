#pragma once

#include <SDL.h>
#include <vector>

namespace flying {

struct Assets;

enum class PipeColor { Green, Blue, Yellow };

class Pipes {
public:
    void init(const Assets& assets);
    void reset();
    void update(float dt, bool moving, int score = 0);
    void draw(SDL_Renderer* renderer, const Assets& assets) const;

    bool hits(float cx, float cy, float radius) const;
    int collectScores(float birdX);

private:
    struct Pair {
        float x = 0;
        float gapCenter = 0;
        float gapSize = 0;
        PipeColor color = PipeColor::Green;
        bool scored = false;
        bool moving = false;
        float baseGapCenter = 0;
        float moveAmplitude = 0;
        float moveSpeed = 0;
        float movePhase = 0;
    };

    void spawn();
    PipeColor nextColor();
    float gapForScore() const;
    void drawPair(SDL_Renderer* renderer, const Assets& assets, const Pair& p) const;
    float scaledW() const;
    float scaledHeadH() const;

    std::vector<Pair> pairs_;
    float spawnX_ = 0;
    int nativeW_ = 326;
    int headH_ = 203;
    int spawnCount_ = 0;
    int greensSinceSpecial_ = 100;
    PipeColor lastSpecial_ = PipeColor::Green;
    int difficultyScore_ = 0;
};

}  // namespace flying
