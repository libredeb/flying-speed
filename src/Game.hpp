#pragma once

#include "Assets.hpp"
#include "Audio.hpp"
#include "Bird.hpp"
#include "Input.hpp"
#include "Parallax.hpp"
#include "Pipes.hpp"
#include "Trail.hpp"

#include <SDL.h>
#include <string>

namespace flying {

class Game {
public:
    bool init(SDL_Renderer* renderer, ProgressFn progress = nullptr);
    void shutdown();

    void handleInput(const Input& input);
    void update(float dt);
    void render();

private:
    enum class State { Title, Playing, Dead };

    void startRun();
    void die();
    void resetToTitle();
    void drawSky() const;
    void drawUi() const;
    void drawText(TTF_Font* font, const std::string& text, int x, int y, SDL_Color color,
                  bool center) const;

    SDL_Renderer* renderer_ = nullptr;
    Assets assets_;
    Audio audio_;
    Bird bird_;
    Trail trail_;
    Pipes pipes_;
    Parallax parallax_;

    State state_ = State::Title;
    int score_ = 0;
    int best_ = 0;
    float deathTimer_ = 0;
    float flash_ = 0;
    bool paused_ = false;
};

}  // namespace flying
