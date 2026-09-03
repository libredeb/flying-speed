#pragma once

#include <SDL.h>

namespace flying {

struct Assets;

class Bird {
public:
    void reset();
    void flap();
    void update(float dt, bool physics);
    void draw(SDL_Renderer* renderer, const Assets& assets) const;

    float x() const { return x_; }
    float y() const { return y_; }
    float vy() const { return vy_; }
    float tailX() const { return x_ - 16.f; }
    float tailY() const { return y_ + 10.f; }

private:
    float x_ = 0;
    float y_ = 0;
    float vy_ = 0;
    float rot_ = 0;
    float hover_ = 0;
};

}  // namespace flying
