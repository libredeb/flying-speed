#pragma once

#include <SDL.h>
#include <array>
#include <cstddef>

namespace flying {

struct Assets;

class Parallax {
public:
    void init(const Assets& assets);
    void reset();
    void update(float dt, bool moving, float speedScale = 1.f);
    void drawBack(SDL_Renderer* renderer) const;
    void drawFront(SDL_Renderer* renderer) const;

private:
    struct Layer {
        SDL_Texture* tex = nullptr;
        float offset = 0;
        float speed = 0;
        int width = 0;
        int height = 0;
    };

    void drawRange(SDL_Renderer* renderer, std::size_t from, std::size_t to) const;

    std::array<Layer, 4> layers_{};
};

}  // namespace flying
