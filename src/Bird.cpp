#include "Bird.hpp"

#include "Assets.hpp"
#include "Config.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cmath>

namespace flying {

void Bird::reset() {
    x_ = kBirdX;
    y_ = static_cast<float>(kLogicalH) * 0.42f;
    vy_ = 0;
    rot_ = 0;
    hover_ = 0;
}

void Bird::flap() {
    vy_ = kFlapVelocity;
}

void Bird::update(float dt, bool physics) {
    if (!physics) {
        hover_ += dt;
        y_ = static_cast<float>(kLogicalH) * 0.42f + std::sin(hover_ * 3.4f) * 14.f;
        rot_ = std::sin(hover_ * 2.2f) * 8.f;
        return;
    }

    vy_ += kGravity * dt;
    vy_ = std::min(vy_, kMaxFallSpeed);
    y_ += vy_ * dt;

    const float t = clampf((vy_ + 380.f) / 980.f, 0.f, 1.f);
    rot_ = lerpf(-28.f, 88.f, t);
}

void Bird::draw(SDL_Renderer* renderer, const Assets& assets) const {
    if (!assets.bird) {
        return;
    }
    SDL_FRect dst{x_ - kBirdDrawSize * 0.5f, y_ - kBirdDrawSize * 0.5f, kBirdDrawSize, kBirdDrawSize};
    SDL_RenderCopyExF(renderer, assets.bird, nullptr, &dst, rot_, nullptr, SDL_FLIP_NONE);
}

}  // namespace flying
