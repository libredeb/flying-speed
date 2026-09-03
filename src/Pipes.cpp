#include "Pipes.hpp"

#include "Assets.hpp"
#include "Config.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cmath>

namespace flying {
namespace {

bool circleAabb(float cx, float cy, float r, float x, float y, float w, float h) {
    const float nx = clampf(cx, x, x + w);
    const float ny = clampf(cy, y, y + h);
    const float dx = cx - nx;
    const float dy = cy - ny;
    return dx * dx + dy * dy < r * r;
}

// Stretch the body in Y: the shaft is a uniform vertical-stripe slice, so one
// blit matches tiling visually and is cheaper (2 copies per column, not N tiles).
void blitPipe(SDL_Renderer* renderer, SDL_Texture* head, SDL_Texture* body, float x, float w,
              float hh, float gapTop, float gapBottom, bool top, float headXOffset) {
    constexpr float kSeam = 2.f;
    const SDL_RendererFlip flip = top ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;

    auto copy = [&](SDL_Texture* tex, const SDL_FRect& dst) {
        SDL_RenderCopyExF(renderer, tex, nullptr, &dst, 0.0, nullptr, flip);
    };

    if (top) {
        const float headY = gapTop - hh;
        SDL_FRect bodyDst{x, 0.f, w, std::max(1.f, headY + kSeam)};
        SDL_FRect headDst{x + headXOffset, headY, w, hh};
        copy(body, bodyDst);
        copy(head, headDst);
    } else {
        const float headY = gapBottom;
        SDL_FRect headDst{x + headXOffset, headY, w, hh};
        const float bodyY = headY + hh - kSeam;
        SDL_FRect bodyDst{x, bodyY, w, std::max(1.f, static_cast<float>(kLogicalH) - bodyY)};
        copy(head, headDst);
        copy(body, bodyDst);
    }
}

}  // namespace

void Pipes::init(const Assets& assets) {
    if (assets.greenHead) {
        SDL_QueryTexture(assets.greenHead, nullptr, nullptr, &nativeW_, &headH_);
    }
}

float Pipes::scaledW() const {
    return static_cast<float>(nativeW_) * kPipeScale;
}

float Pipes::scaledHeadH() const {
    return static_cast<float>(headH_) * kPipeScale;
}

void Pipes::reset() {
    pairs_.clear();
    spawnX_ = static_cast<float>(kLogicalW) + 90.f;
    spawnCount_ = 0;
    greensSinceSpecial_ = 100;
    lastSpecial_ = PipeColor::Green;
    difficultyScore_ = 0;
}

PipeColor Pipes::nextColor() {
    ++spawnCount_;
    if (spawnCount_ <= kOpeningGreenPipes || greensSinceSpecial_ < kMinGreensBetweenSpecials) {
        ++greensSinceSpecial_;
        return PipeColor::Green;
    }
    // Specials are occasional; skip most of the time.
    if (randInt(0, 99) >= 20) {
        ++greensSinceSpecial_;
        return PipeColor::Green;
    }
    PipeColor special = PipeColor::Blue;
    if (lastSpecial_ == PipeColor::Blue) {
        special = PipeColor::Yellow;
    } else if (lastSpecial_ == PipeColor::Yellow) {
        special = PipeColor::Blue;
    } else {
        special = (randInt(0, 1) == 0) ? PipeColor::Blue : PipeColor::Yellow;
    }
    lastSpecial_ = special;
    greensSinceSpecial_ = 0;
    return special;
}

float Pipes::gapForScore() const {
    const float t = clampf(static_cast<float>(difficultyScore_) / static_cast<float>(kGapDifficultyScore),
                           0.f, 1.f);
    const float u = t * t;
    const float lo = lerpf(kGapEasyMin, kGapHardMin, u);
    const float hi = lerpf(kGapEasyMax, kGapHardMax, u);
    const int roll = randInt(0, 99);
    if (roll < 16) {
        return randFloat(lerpf(lo, hi, 0.7f), hi + lerpf(36.f, 10.f, u));
    }
    if (roll < 38) {
        return randFloat(lo, lerpf(lo, hi, 0.32f));
    }
    return randFloat(lo, hi);
}

void Pipes::spawn() {
    Pair p;
    p.gapSize = gapForScore();
    const float minCenter = kGapMargin + p.gapSize * 0.5f;
    const float maxCenter = static_cast<float>(kLogicalH) - kGapMargin - p.gapSize * 0.5f;
    p.gapCenter = randFloat(minCenter, maxCenter);
    p.baseGapCenter = p.gapCenter;
    p.color = nextColor();
    p.x = spawnX_;
    p.scored = false;

    const int allowedMovers = (kMovingPipeInterval > 0 && difficultyScore_ >= kMovingPipeInterval)
                                  ? std::min(difficultyScore_ / kMovingPipeInterval, kMaxMovingPipes)
                                  : 0;
    if (allowedMovers > 0) {
        int currentMovers = 0;
        for (const auto& ep : pairs_) {
            if (ep.moving) ++currentMovers;
        }
        if (currentMovers < allowedMovers) {
            p.moving = true;
            p.moveSpeed = randFloat(kMovingPipeSpeedMin, kMovingPipeSpeedMax);
            p.moveAmplitude = randFloat(kMovingPipeAmpMin, kMovingPipeAmpMax);
            p.movePhase = randFloat(0.f, 6.2831853f);

            const float ampMargin = p.moveAmplitude + p.gapSize * 0.5f + kGapMargin;
            p.baseGapCenter = clampf(p.baseGapCenter, ampMargin,
                                     static_cast<float>(kLogicalH) - ampMargin);
            p.gapCenter = p.baseGapCenter + std::sin(p.movePhase) * p.moveAmplitude;
        }
    }

    pairs_.push_back(p);
    spawnX_ += randFloat(kPipeMinSpacing, kPipeMaxSpacing);
}

void Pipes::update(float dt, bool moving, int score) {
    difficultyScore_ = score;
    if (!moving) {
        return;
    }

    const float dx = kScrollSpeed * dt;
    spawnX_ -= dx;
    for (auto& p : pairs_) {
        p.x -= dx;
        if (p.moving) {
            p.movePhase += p.moveSpeed * dt;
            p.gapCenter = p.baseGapCenter + std::sin(p.movePhase) * p.moveAmplitude;
        }
    }

    const float width = scaledW();
    pairs_.erase(std::remove_if(pairs_.begin(), pairs_.end(),
                                [width](const Pair& p) { return p.x + width < -8.f; }),
                 pairs_.end());

    while (spawnX_ < static_cast<float>(kLogicalW) + 8.f) {
        spawn();
    }
}

void Pipes::drawPair(SDL_Renderer* renderer, const Assets& assets, const Pair& p) const {
    SDL_Texture* head = assets.greenHead;
    SDL_Texture* body = assets.greenBody;
    switch (p.color) {
        case PipeColor::Blue:
            head = assets.blueHead;
            body = assets.blueBody;
            break;
        case PipeColor::Yellow:
            head = assets.yellowHead;
            body = assets.yellowBody;
            break;
        case PipeColor::Green:
            break;
    }
    const float gapTop = p.gapCenter - p.gapSize * 0.5f;
    const float gapBottom = p.gapCenter + p.gapSize * 0.5f;
    const float headXOffset =
        (p.color == PipeColor::Blue) ? kBlueHeadAlignPx * kPipeScale : 0.f;
    blitPipe(renderer, head, body, p.x, scaledW(), scaledHeadH(), gapTop, gapBottom, true,
             headXOffset);
    blitPipe(renderer, head, body, p.x, scaledW(), scaledHeadH(), gapTop, gapBottom, false,
             headXOffset);
}

void Pipes::draw(SDL_Renderer* renderer, const Assets& assets) const {
    for (const auto& p : pairs_) {
        drawPair(renderer, assets, p);
    }
}

bool Pipes::hits(float cx, float cy, float radius) const {
    const float w = scaledW();
    const float headH = scaledHeadH();
    for (const auto& p : pairs_) {
        const float gapTop = p.gapCenter - p.gapSize * 0.5f;
        const float gapBottom = p.gapCenter + p.gapSize * 0.5f;
        const float headX = p.x + w * kHeadPadL;
        const float headW = w * (1.f - kHeadPadL - kHeadPadR);
        const float bodyX = p.x + w * kBodyPadL;
        const float bodyW = w * (1.f - kBodyPadL - kBodyPadR);

        if (circleAabb(cx, cy, radius, headX, gapTop - headH, headW, headH)) {
            return true;
        }
        if (circleAabb(cx, cy, radius, bodyX, 0.f, bodyW, std::max(0.f, gapTop - headH))) {
            return true;
        }
        if (circleAabb(cx, cy, radius, headX, gapBottom, headW, headH)) {
            return true;
        }
        if (circleAabb(cx, cy, radius, bodyX, gapBottom + headH, bodyW,
                       std::max(0.f, static_cast<float>(kLogicalH) - (gapBottom + headH)))) {
            return true;
        }
    }
    return false;
}

int Pipes::collectScores(float birdX) {
    int gained = 0;
    const float mid = scaledW() * 0.55f;
    for (auto& p : pairs_) {
        if (!p.scored && birdX > p.x + mid) {
            p.scored = true;
            ++gained;
        }
    }
    return gained;
}

}  // namespace flying
