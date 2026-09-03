#include "Trail.hpp"

#include "Util.hpp"

#include <algorithm>

namespace flying {

void Trail::reset() {
    particles_.clear();
}

void Trail::update(float dt) {
    for (auto& p : particles_) {
        p.life -= dt;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vx *= 0.90f;
        p.vy *= 0.95f;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const Particle& p) { return p.life <= 0.f; }),
                     particles_.end());
}

void Trail::emitContinuous(float x, float y, float vy) {
    for (int i = 0; i < 4; ++i) {
        Particle p;
        p.x = x - randFloat(4.f, 20.f);
        p.y = y + randFloat(-9.f, 11.f);
        p.vx = randFloat(-130.f, -45.f);
        p.vy = vy * 0.10f + randFloat(-20.f, 20.f);
        p.maxLife = randFloat(0.29f, 0.52f);
        p.life = p.maxLife;
        p.size = randFloat(18.f, 31.f);
        particles_.push_back(p);
    }
}

void Trail::emitFlap(float x, float y) {
    const int burst = randInt(10, 16);
    for (int i = 0; i < burst; ++i) {
        Particle p;
        p.x = x - randFloat(2.f, 24.f);
        p.y = y + randFloat(-14.f, 16.f);
        p.vx = randFloat(-210.f, -55.f);
        p.vy = randFloat(-60.f, 80.f);
        p.maxLife = randFloat(0.31f, 0.55f);
        p.life = p.maxLife;
        p.size = randFloat(21.f, 36.f);
        particles_.push_back(p);
    }
}

void Trail::draw(SDL_Renderer* renderer, SDL_Texture* dot) const {
    if (!dot) {
        return;
    }
    for (const auto& p : particles_) {
        const float t = clampf(p.life / p.maxLife, 0.f, 1.f);
        SDL_SetTextureAlphaMod(dot, static_cast<Uint8>(t * t * 255.f));
        const float w = p.size * (0.85f + (1.f - t) * 1.43f);
        const float h = p.size * (0.50f + t * 0.30f);
        SDL_FRect dst{p.x - w * 0.5f, p.y - h * 0.5f, w, h};
        SDL_RenderCopyF(renderer, dot, nullptr, &dst);
    }
    SDL_SetTextureAlphaMod(dot, 255);
}

}  // namespace flying
