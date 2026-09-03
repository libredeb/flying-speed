#pragma once

#include <SDL.h>
#include <vector>

namespace flying {

class Trail {
public:
    void reset();
    void update(float dt);
    void emitContinuous(float x, float y, float vy);
    void emitFlap(float x, float y);
    void draw(SDL_Renderer* renderer, SDL_Texture* dot) const;

private:
    struct Particle {
        float x = 0;
        float y = 0;
        float vx = 0;
        float vy = 0;
        float life = 0;
        float maxLife = 1;
        float size = 8;
    };

    std::vector<Particle> particles_;
};

}  // namespace flying
